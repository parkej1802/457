#include "LineGeometry.h"

#include <VulkanLaunchpad.h>
#include <vulkan/vulkan.hpp>

#include "LinePipeline.hpp"

// A vector to organize geometry on the GPU.
// Also need a buffer on the GPU to copy the geometry data to.

uint32_t mNumLineVertices;
std::vector<glm::vec3> vertices;
VkBuffer mLineVertices;

// If using indexed drawing, the following can be used.
// uint32_t mNumLineIndices;
// std::vector<glm::vec3> indices;
// VkBuffer mLineIndices;

// tx and ty keep track of wrap around
extern float tx;
extern float ty;

// dx and dy are used as incremental translations
extern float dx;
extern float dy;

int n = 3;

std::shared_ptr<MyApp::LinePipeline> linePipeline{};

/**
 * Create line geometry that is to be drawn. 
 * Allocate a buffer on the GPU that is large enough to hold the 
 * geometry data. Copy date over to the GPU.
 * 
 * For this starter example, we'll create vertices for the polar plot:
 * r = 0.8 * cos(2*t), 0 <= t < 2*Pi
 */

//rotate/flip a quadrant appropriately
void rot(int n, int *x, int *y, int rx, int ry) {
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }

        //Swap x and y
        int t  = *x;
        *x = *y;
        *y = t;
    }
}

//convert (x,y) to d
int xy2d (int n, int x, int y) {
    int rx, ry, s, d=0;
    for (s=n/2; s>0; s/=2) {
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);
        rot(n, &x, &y, rx, ry);
    }
    return d;
}

//convert d to (x,y)
void d2xy(int n, int d, int *x, int *y) {
    int rx, ry, s, t=d;
    *x = *y = 0;
    for (s=1; s<n; s*=2) {
        rx = 1 & (t/2);
        ry = 1 & (t ^ rx);
        rot(s, x, y, rx, ry);
        *x += s * rx;
        *y += s * ry;
        t /= 4;
    }
}
 
void lineInitGeometryAndBuffers() {
  
  //mNumLineVertices = 4;
  // mNumLineIndices = 255;
  
  //vertices = std::vector<glm::vec3>( mNumLineVertices ) ;
  // for (size_t i = 0; i < mNumLineVertices; i++) {
  //   float theta =  2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(mNumLineVertices-1);
  //   float r = 0.8 * glm::cos(2.0f * theta);
  //   glm::vec3 vert = glm::vec3(r * glm::cos(theta), r * glm::sin(theta), 1.0f);
  //   vertices[i] = vert; 
  
  
  mNumLineVertices = pow(4,n);
  vertices = std::vector<glm::vec3>(mNumLineVertices);
  int d = sqrt(mNumLineVertices);

  float scale = static_cast<float>(d) - 1.0f;

  for (size_t i = 0; i < mNumLineVertices; i++) {
    int x, y;
    d2xy(d, i, &x, &y);
    
    float xn = 2.0f * (float)x / scale - 1.0f;
    float yn = 2.0f * (float)y / scale - 1.0f;
      
    vertices[i] = glm::vec3(xn, yn, 1.0f);
  }



  linePipeline = std::make_shared<MyApp::LinePipeline>();

  // Need indices if using indexed drawing.

  // std::vector<unsigned int> indices;
  // for(unsigned int i = 0; i < mNumLineIndices; i++) {
  //   indices.push_back(i);
  // }
  
  // Create vertex buffer on GPU and copy data into it.
  mLineVertices = vklCreateHostCoherentBufferAndUploadData(
      vertices.data(), sizeof(vertices[0]) * vertices.size(),
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

  // Need to create an index buffer if using indexed drawing
  //mLineIndices = vklCreateHostCoherentBufferAndUploadData(
  //    indices.data(), sizeof(indices[0]) * indices.size(),
  //    VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

 //   pipeline = vklCreateGraphicsPipeline(VklGraphicsPipelineConfig{
	//	// Vertex Shader from memory:
	//		"#version 450\n"
	//		"layout(location = 0) in vec3 position;\n"
	//		"void main() {\n"
	//		"    gl_Position = vec4(position.x, -position.y, position.z, 1);\n"
	//		"}\n",
	//	// Fragment shader from memory:
	//		"#version 450\n"
	//		"layout(location = 0) out vec4 color; \n"
	//		"void main() {  \n"
	//		"    color = vec4(1, 0, 0, 1); \n"
	//		"}\n",
	//	// Further config parameters:
	//	{
	//		VkVertexInputBindingDescription { 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX }
	//	},
	//	{
	//		VkVertexInputAttributeDescription { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0u }
	//	},
	//	VK_POLYGON_MODE_FILL,
	//	VK_CULL_MODE_NONE,
	//	{ /* no descriptors */ }
	//}, /* load shaders from memory: */ true, PrimitiveTopology::eLineList);
}

/**
 * Update geomemtry by applying transformations. Copy updated
 * geometry to the GPU.
 */
void lineUpdateGeometryAndBuffers() {
  
  // Any transformations to the vertices can be applied as
  // follows:
  // (glm stores matrices in column-major order)
  // A simple translation for now that can be controlled
  // via the cursor keys.
  
  auto T = glm::mat3(1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  dx, dy, 1.0f);

  for (size_t i = 0; i < vertices.size(); i++) {
    vertices[i] = T * vertices[i];
  }

  vklCopyDataIntoHostCoherentBuffer(mLineVertices, vertices.data(), 
    sizeof(vertices[0]) * vertices.size());
}


/**
 * Cleanup buffers created on the GPU that are no longer
 * needed.
 */
void lineDestroyBuffers() {
  auto device = vklGetDevice();
  vklDestroyHostCoherentBufferAndItsBackingMemory( mLineVertices );
  linePipeline.reset();
	// vklDestroyHostCoherentBufferAndItsBackingMemory( mLineIndices ); 
}

/**
 * Call this from within a render loop to draw.
 * To draw line strips using the basic graphics pipeline provided by 
 * VulkanLaunchpad, please make the following change in the basic graphics
 * pipeline before calling this function:
 * 
 * Set the appropriate primitive topology in the body of
 * vklCreateGraphicsPipeline() function. 
 * For example, to draw line strips use the following:
 *   
 *   ...setTopology(vk::PrimitiveTopology::eLineStrip) 
 */
void lineDraw() {
  if (!vklFrameworkInitialized()) {
    VKL_EXIT_WITH_ERROR(
        "Framework not initialized. Ensure to invoke vklFrameworkInitialized "
        "beforehand!");
  }

  const vk::CommandBuffer& cb = vklGetCurrentCommandBuffer();
  auto currentSwapChainImageIndex = vklGetCurrentSwapChainImageIndex();
  assert(currentSwapChainImageIndex < vklGetNumFramebuffers());
  assert(currentSwapChainImageIndex < vklGetNumClearValues());

  cb.bindPipeline( vk::PipelineBindPoint::eGraphics, linePipeline->GetPipeline());
  
  cb.bindVertexBuffers(0u, {mLineVertices}, {vk::DeviceSize{0}});
  //cb.bindIndexBuffer(vk::Buffer{ mLineIndices }, vk::DeviceSize{ 0 }, vk::IndexType::eUint32);
  cb.draw(mNumLineVertices, 1u, 0u, 0u);
  //cb.drawIndexed(mNumLineIndices, 1u, 0u, 0, 0u);
}

VkBuffer lineGetVerticesBuffer() {
  return static_cast<VkBuffer>(mLineVertices);
}
