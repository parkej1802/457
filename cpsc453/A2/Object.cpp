/*
 * Copyright 2023 University of Calgary, Visualization and Graphics Group
 */

#include "Object.h"
#include <VulkanLaunchpad.h>
#include <vulkan/vulkan.hpp>
#include <random>
#include "Camera.h"


// buffers that will live on the GPU.
// No geometry retained on the CPU, all data sent to the GPU.

uint32_t mNumObjectIndices;
VkBuffer mObjectVertexData;
VkBuffer mObjectIndices;

// A pipeline that can be used for HW2
VkPipeline pipeline;
extern VklCameraHandle camera;

// Struct to pack object vertex data
struct Vertex {
	glm::vec3 position;
    glm::vec3 normal;
}; 

// Send model, view and projection matrices as push constants
// which are packed in this struct
struct ObjectPushConstants {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

};

// MVP matrices that are updated interactively
ObjectPushConstants pushConstants;

// Simple interactive rotation of object controlled by an angle

//initialized values in Main.cpp to use in Object.cpp
extern float angle;
extern float angleIX;
extern float angleIY;
extern float angleIZ;
extern float angleEX;
extern float angleEY;
extern float angleEZ;
extern float scaleFactor;
extern float scaleUpdated;
extern bool Extrinsic;


glm::vec3 calculateTriangleNormal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
	return glm::normalize(glm::cross(v1 - v0, v2 - v0));
}

float Xmax = 0;
float Ymax = 0;
float Zmax = 0;
float Xmin = 0;
float Ymin = 0;
float Zmin = 0;


    

// Organize geometry data and send it to the GPU 
void objectCreateGeometryAndBuffers() 
{

	auto loadFile = vklLoadModelGeometry("../HW2/models/airplane.obj");

	std::vector<glm::vec3> positions = loadFile.positions;
	std::vector<unsigned int> indices = loadFile.indices;
	

	glm::vec3 minBounds = positions[0];
    glm::vec3 maxBounds = positions[0];
    for(const glm::vec3& pos : positions) {
        minBounds = glm::min(minBounds, pos);
        maxBounds = glm::max(maxBounds, pos);
    }

    // Update global bounds variables
    Xmin = minBounds.x;
    Ymin = minBounds.y;
    Zmin = minBounds.z;
    Xmax = maxBounds.x;
    Ymax = maxBounds.y;
    Zmax = maxBounds.z;

	float centerX = (Xmin + Xmax) / 2.0f;
    float centerY = (Ymin + Ymax) / 2.0f;
    float centerZ = (Zmin + Zmax) / 2.0f;

	float center = glm::length(glm::vec3(Xmax - centerX, Ymax - centerY, Zmax - centerZ));
	float scalingFactor = 2.0f/center;

	glm::mat4 matScale = glm::scale(glm::mat4(1.0f), glm::vec3(scalingFactor, scalingFactor, scalingFactor));
	glm::mat4 scaleAndTranslate = glm::translate(matScale, glm::vec3(-centerX, -centerY, -centerZ));

	for(unsigned int i = 0; i < positions.size(); i++){
		positions[i] = glm::vec3(scaleAndTranslate * glm::vec4(positions[i].x, positions[i].y,positions[i].z, 1.0f));
	}


	std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f, 0.0f, 0.0f));

	for (unsigned int i = 0; i < indices.size(); i += 3) {
		
		unsigned int idx0 = indices[i];
		unsigned int idx1 = indices[i + 1];
		unsigned int idx2 = indices[i + 2];

		glm::vec3 normal = calculateTriangleNormal(positions[idx0], positions[idx1], positions[idx2]);

		normals[idx0] += normal;
		normals[idx1] += normal;
		normals[idx2] += normal;
	}

	for (unsigned int i = 0; i < normals.size(); i++) {
		normals[i] = glm::normalize(normals[i]);
	}

	

	// Create a vector to interleave and pack all vertex data into one vector.
	std::vector<Vertex> vData( positions.size() );
	for( unsigned int i = 0; i < vData.size(); i++ ) {
		vData[i].position = positions[i];
		vData[i].normal = normals[i];
	}
	
	

	mNumObjectIndices = static_cast<uint32_t>(indices.size());
	const auto device = vklGetDevice();
	auto dispatchLoader = vk::DispatchLoaderStatic();

	// 1. Vertex BUFFER (Buffer, Memory, Bind 'em together, copy data into it)
	{ 
		// Use VulkanLaunchpad functionality to manage buffers
		// All vertex data is in one vector, copied to one buffer
		// on the GPU
		mObjectVertexData = vklCreateHostCoherentBufferAndUploadData(
			vData.data(), sizeof(vData[0]) * vData.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}

	// 2. INDICES BUFFER (Buffer, Memory, Bind 'em together, copy data into it)
	{
		mObjectIndices = vklCreateHostCoherentBufferAndUploadData(
			indices.data(), sizeof(indices[0]) * indices.size(),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}

	// Now Create the pipeline
	objectCreatePipeline();
}


// Cleanup buffers and pipeline created on the GPU 
void objectDestroyBuffers() {
	auto device = vklGetDevice();
	vkDeviceWaitIdle( device );
	vklDestroyGraphicsPipeline(pipeline);
	vklDestroyHostCoherentBufferAndItsBackingMemory( mObjectVertexData );
	vklDestroyHostCoherentBufferAndItsBackingMemory( mObjectIndices );
}

void objectDraw() {
	objectDraw( pipeline );
}

void objectDraw(VkPipeline pipeline)
{
	if (!vklFrameworkInitialized()) {
		VKL_EXIT_WITH_ERROR("Framework not initialized. Ensure to invoke vklFrameworkInitialized beforehand!");
	}
	const vk::CommandBuffer& cb = vklGetCurrentCommandBuffer();
	auto currentSwapChainImageIndex = vklGetCurrentSwapChainImageIndex();
	assert(currentSwapChainImageIndex < vklGetNumFramebuffers());
	assert(currentSwapChainImageIndex < vklGetNumClearValues());

	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	cb.bindVertexBuffers(0u, { vk::Buffer{ objectGetVertexBuffer() } }, { vk::DeviceSize{ 0 } });
	cb.bindIndexBuffer(vk::Buffer{ objectGetIndicesBuffer() }, vk::DeviceSize{ 0 }, vk::IndexType::eUint32);

	// update push constants on every draw call and send them over to the GPU.
    // upload the matrix to the GPU via push constants
	objectUpdateConstants();
    vklSetPushConstants(
			pipeline, 
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
			&pushConstants, 
			sizeof(ObjectPushConstants)
		);

	cb.drawIndexed(objectGetNumIndices(), 1u, 0u, 0, 0u);
}

VkBuffer objectGetVertexBuffer() {
	return static_cast<VkBuffer>(mObjectVertexData);
}

VkBuffer objectGetIndicesBuffer() {
	return static_cast<VkBuffer>(mObjectIndices);
}

uint32_t objectGetNumIndices() {
	return mNumObjectIndices;
}

void objectCreatePipeline() {

	// initialize push constants
	pushConstants.model = glm::mat4{ 1.0f };

	// a right-handed view coordinate system coincident with the x y and z axes
	// and located along the positive z axis, looking down the negative z axis.
	glm::mat4 view = glm::mat4{
		glm::vec4{ 1.f,  0.f,  0.f,  0.f},
		glm::vec4{ 0.f,  1.f,  0.f,  0.f},
		glm::vec4{ 0.f,  0.f,  1.f,  0.f},
		glm::vec4{ 0.f,  0.25f,  2.f,  1.f},
	};
	pushConstants.view = glm::inverse( view );
	
	// Create a projection matrix compatible with Vulkan.
	// The resulting matrix takes care of the y-z flip.
	pushConstants.proj = vklCreatePerspectiveProjectionMatrix(glm::pi<float>() / 3.0f, 1.0f, 1.0f, 3.0f );

	// ------------------------------
	// Pipeline creation
	// ------------------------------

	VklGraphicsPipelineConfig config{};
		config.enableAlphaBlending = false;
		// path to shaders may need to be modified depending on the location
		// of the executable
		config.vertexShaderPath = "../HW2/src/starter.vert";
		config.fragmentShaderPath = "../HW2/src/starter.frag";
		
		// Can set polygonDrawMode to VK_POLYGON_MODE_LINE for wireframe rendering
		config.polygonDrawMode = VK_POLYGON_MODE_FILL;
		config.triangleCullingMode = VK_CULL_MODE_BACK_BIT;

		// Binding for vertex buffer, using 1 buffer with per-vertex rate.
		// This will send per-vertex data to the GPU.
		config.vertexInputBuffers.emplace_back(VkVertexInputBindingDescription{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		});

		// Positions at locaion 0
		config.inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			//.location = static_cast<uint32_t>(config.inputAttributeDescriptions.size()),
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, position),
		});

		// Normals at location 1
		config.inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			//.location = static_cast<uint32_t>(config.inputAttributeDescriptions.size()),
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, normal),
		});
            
		// Push constants should be available in both the vertex and fragment shaders
		config.pushConstantRanges.emplace_back(VkPushConstantRange{
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = sizeof(ObjectPushConstants),
		});
	pipeline = vklCreateGraphicsPipeline( config );		
}

// Function to update push constants.
// For the starter example, only the model matrix is updated.
void objectUpdateConstants() {

//if 
	if (Extrinsic == true) {
		pushConstants.model = glm::rotate(glm::mat4(1.0f), angleEX, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), angleEY, glm::vec3(0.0f, 1.0f, 0.0f)) * 
		glm::rotate(glm::mat4(1.0f), angleEZ, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else {
		if(angleIX != 0.0f) {
			pushConstants.model = glm::rotate(pushConstants.model, angleIX, glm::vec3(1.0f, 0.0f, 0.0f));
			angleIX = 0.0f; 
		}
		
		else if (angleIY != 0.0f) {
			pushConstants.model = glm::rotate(pushConstants.model, angleIY, glm::vec3(0.0f, 1.0f, 0.0f));
			angleIY = 0.0f;
		}	
		
		else if (angleIZ != 0.0f) {
			pushConstants.model = glm::rotate(pushConstants.model, angleIZ, glm::vec3(0.0f, 0.0f, 1.0f));
			angleIZ = 0.0f;
		}

	}

	if (scaleUpdated) {
		pushConstants.model = glm::scale(pushConstants.model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
		scaleUpdated = false; 
	}

	

	pushConstants.view = vklGetCameraViewMatrix(camera);
	pushConstants.proj = vklGetCameraProjectionMatrix(camera);
}