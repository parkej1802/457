#include "BedrockLog.hpp"
#include "BedrockPath.hpp"
#include "LogicalDevice.hpp"
#include "pipeline/LinePipeline.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "BedrockMath.hpp"
#include "BufferTracker.hpp"
#include "camera/PerspectiveCamera.hpp"
#include "utils/LineRenderer.hpp"
#include "UI.hpp"
#include "pipeline/FlatShadingPipeline.hpp"
#include "ImportTexture.hpp"
#include "camera/ObserverCamera.hpp"
#include "utils/PointRenderer.hpp"
#include "camera/ArcballCamera.hpp"

#include <glm/glm.hpp>
#include <filesystem>

#include "ImportObj.hpp"

using namespace MFA;

// initialize variables 
bool renderWireframe = false;
bool perlinNoise = false;
int m = 24;
int perlinNoiseInt = 0;
bool AOTexture = false;
bool texture = false;
int AOTextureInt = 0;
int textureInt = 0;
float PI = 3.14159;
float x = 0;
float y = 0;
float z = PI;
float scaleF = 0;


std::string fileName;
std::string fileNameAO;

void UI_Loop()
{
	if (!perlinNoise) {
		perlinNoiseInt = 0;
	}
	else {
		perlinNoiseInt = 1;
	}

	if (!AOTexture) {
		AOTextureInt = 0;
	}
	else {
		AOTextureInt = 1;
	}

	if (!texture) {
		textureInt = 0;
	}
	else {
		textureInt = 1;
	}

	//GUI CONTROLLER
	auto ui = UI::Instance;
	ui->BeginWindow("Settings");
	ImGui::Checkbox("Wireframe enable", &renderWireframe);
	ImGui::Checkbox("Perlin Noise enable", &perlinNoise);
	ImGui::SliderInt("Control m", &m, 24, 48);
	ImGui::Checkbox("AO enable",&AOTexture);
	ImGui::Checkbox("Texture enable", &texture);
	ImGui::SliderFloat("X", &x, -PI, PI);
	ImGui::SliderFloat("Y", &y, -PI, PI);
	ImGui::SliderFloat("Z", &z, -PI, PI);
	ui->EndWindow();
};

class FlagMesh
{
public:

	using Triangle = std::tuple<int, int, int>;

	explicit FlagMesh(
		std::shared_ptr<FlatShadingPipeline> pipeline,
		std::shared_ptr<FlatShadingPipeline> wireframePipeline,
		glm::vec4 const& color,
		std::vector<glm::vec3> const& vertices,
		std::vector<Triangle> const& triangles,         // Indices and intensity
		std::vector<glm::vec2> const & uvs,
		std::vector<glm::vec3> const & normals
	)
		: _pipeline(std::move(pipeline))
		, _wireframePipeline(std::move(wireframePipeline))
		, _color(color)
	{

		CreateIndexBuffer(triangles);

		CreateVertexBuffer(vertices, uvs, normals);
		
		CreateGpuTexture();

		CreateMaterial();

		CreateDescriptorSet();
	}

	void Render(RT::CommandRecordState& recordState)
	{
		if (renderWireframe == true)
		{
			_wireframePipeline->BindPipeline(recordState);
		}
		else
		{
			_pipeline->BindPipeline(recordState);
		}
		
		RB::AutoBindDescriptorSet(
			recordState, 
			RB::UpdateFrequency::PerGeometry, 
			_perGeometryDescriptorSet.descriptorSets[0]
		);


		// Rotate x, y z axis
		glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), x, glm::vec3{1.0f, 0.0f, 0.0f});
		glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), y, glm::vec3{0.0f, 1.0f, 0.0f});
		glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), z, glm::vec3{0.0f, 0.0f, 1.0f});

		glm::mat4 rotateModel = rotateX * rotateY * rotateZ;

		// push data
		_pipeline->SetPushConstants(
			recordState,
			FlatShadingPipeline::PushConstants{
				.model = rotateModel,
				.perlinNoiseInt = perlinNoiseInt,
				.m = m,
				.AOTextureInt = AOTextureInt,
				.textureInt = texture

			}
		);
		
		RB::BindIndexBuffer(
			recordState,
			*_indexBuffer,
			0,
			VK_INDEX_TYPE_UINT16
		);

		RB::BindVertexBuffer(
			recordState,
			*_vertexBuffer,
			0,
			0
		);

		RB::DrawIndexed(
			recordState,
			_indexCount
		);
	}

private:

	void CreateIndexBuffer(std::vector<Triangle> const& triangles)
	{
		std::vector<uint16_t> indices{};
		for (auto& [idx1, idx2, idx3] : triangles)
		{
			indices.emplace_back(idx1);
			indices.emplace_back(idx2);
			indices.emplace_back(idx3);
		}
		Alias const indicesAlias{ indices.data(), indices.size() };

		auto const* device = LogicalDevice::Instance;

		auto const commandBuffer = RB::BeginSingleTimeCommand(
			device->GetVkDevice(),
			device->GetGraphicCommandPool()
		);

		auto const indexStageBuffer = RB::CreateStageBuffer(
			device->GetVkDevice(),
			device->GetPhysicalDevice(),
			indicesAlias.Len(),
			1
		);

		_indexBuffer = RB::CreateIndexBuffer(
			device->GetVkDevice(),
			device->GetPhysicalDevice(),
			commandBuffer,
			*indexStageBuffer->buffers[0],
			indicesAlias
		);

		RB::EndAndSubmitSingleTimeCommand(
			device->GetVkDevice(),
			device->GetGraphicCommandPool(),
			device->GetGraphicQueue(),
			commandBuffer
		);

		_indexCount = static_cast<int>(indices.size());
	}

	void CreateVertexBuffer(std::vector<glm::vec3> const & positions, std::vector<glm::vec2> const & uvs, std::vector<glm::vec3> const & normals)
	{
		auto device = LogicalDevice::Instance->GetVkDevice();
		auto physicalDevice = LogicalDevice::Instance->GetPhysicalDevice();
		auto graphicCommandPool = LogicalDevice::Instance->GetGraphicCommandPool();
		auto graphicQueue = LogicalDevice::Instance->GetGraphicQueue();

		auto const commandBuffer = RB::BeginSingleTimeCommand(
			device,
			graphicCommandPool
		);

		// Vertices and normals should be updated every frame
		std::vector<FlatShadingPipeline::Vertex> vertices(positions.size());
		for (int i = 0; i < static_cast<int>(positions.size()); ++i)
		{
			vertices[i].position = positions[i];
			vertices[i].baseColorUV = uvs[i];
			vertices[i].normal = normals[i];
		}

		Alias const alias{ vertices.data(), vertices.size() };

		auto const stageBuffer = RB::CreateStageBuffer(
			device,
			physicalDevice,
			alias.Len(),
			1
		);

		_vertexBuffer = RB::CreateVertexBuffer(
			device,
			physicalDevice,
			commandBuffer,
			*stageBuffer->buffers[0],
			alias
		);

		RB::EndAndSubmitSingleTimeCommand(
			device,
			graphicCommandPool,
			graphicQueue,
			commandBuffer
		);
	}

	void CreateGpuTexture()
	{
		//get the path of AO file and texture file and save each data in separate buffer
		auto const path = Path::Instance->Get("models/chess_bishop/" + fileNameAO);
		MFA_ASSERT(std::filesystem::exists(path));
		auto const cpuTexture = Importer::UncompressedImage(path);

		auto path1 = Path::Instance->Get("models/chess_bishop/" + fileName);
		MFA_ASSERT(std::filesystem::exists(path1));
		auto const cpuTexture1 = Importer::UncompressedImage(path1);
	
		int width = 24;
		int height = 24;
		int components = 4;
		auto const blob = Memory::AllocSize(width * height * components);
		auto* ptr = blob->As<uint8_t>();
		for (int i = 0; i < width * height * components; ++i)
		{
			ptr[i] = Math::Random(0, 256); 
		}

		auto inMemoryTexture = Importer::InMemoryTexture(
			*blob, 
			width,
			height, 
			Asset::Texture::Format::UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR,
			components
		);


		auto const* device = LogicalDevice::Instance;
		auto const commandBuffer = RB::BeginSingleTimeCommand(
			device->GetVkDevice(),
			device->GetGraphicCommandPool()
		);

		// Creating texture separating so we can now add texture on top of another textures
		auto [texture, stagingBuffer] = RB::CreateTexture(
			*cpuTexture1,
			LogicalDevice::Instance->GetVkDevice(),
			LogicalDevice::Instance->GetPhysicalDevice(),
			commandBuffer
		);

		auto [AOtexture, AOstagingBuffer] = RB::CreateTexture(
			*cpuTexture,
			LogicalDevice::Instance->GetVkDevice(),
			LogicalDevice::Instance->GetPhysicalDevice(),
			commandBuffer
		);

		auto [Perlintexture, PerlinstagingBuffer] = RB::CreateTexture(
			*inMemoryTexture,
			LogicalDevice::Instance->GetVkDevice(),
			LogicalDevice::Instance->GetPhysicalDevice(),
			commandBuffer
		);

		//save variable
		_texture = texture;
		_AOtexture = AOtexture;
		_Perlintexture = Perlintexture;

		RB::EndAndSubmitSingleTimeCommand(
			device->GetVkDevice(),
			device->GetGraphicCommandPool(),
			device->GetGraphicQueue(),
			commandBuffer
		);
	}


	void CreateMaterial()
	{
		FlatShadingPipeline::Material material{
			.color = _color,
			.hasBaseColorTexture = 1
		};

		_material = RB::CreateLocalUniformBuffer(
			LogicalDevice::Instance->GetVkDevice(),
			LogicalDevice::Instance->GetPhysicalDevice(),
			sizeof(material),
			1
		);

		auto stageBuffer = RB::CreateStageBuffer(
			LogicalDevice::Instance->GetVkDevice(), 
			LogicalDevice::Instance->GetPhysicalDevice(), 
			sizeof(material), 
			1
		);

		auto commandBuffer = RB::BeginSingleTimeCommand(
			LogicalDevice::Instance->GetVkDevice(), 
			LogicalDevice::Instance->GetGraphicCommandPool()
		);

		RB::UpdateHostVisibleBuffer(
			LogicalDevice::Instance->GetVkDevice(), 
			*stageBuffer->buffers[0], 
			Alias(material)
		);

		RB::UpdateLocalBuffer(
			commandBuffer, 
			*_material->buffers[0], 
			*stageBuffer->buffers[0]
		);

		RB::EndAndSubmitSingleTimeCommand(
			LogicalDevice::Instance->GetVkDevice(),
			LogicalDevice::Instance->GetGraphicCommandPool(),
			LogicalDevice::Instance->GetGraphicQueue(),
			commandBuffer
		);
	}

	void CreateDescriptorSet()
	{	
		// Edited the CreatePerGeometryDescriptorSetGroup by adding two new parameter of ao and perlin textures
		_perGeometryDescriptorSet = _pipeline->CreatePerGeometryDescriptorSetGroup(*_material->buffers[0], *_texture, *_AOtexture, *_Perlintexture);
		//_perGeometryDescriptorSet = _pipeline->CreatePerGeometryDescriptorSetGroup(*_material->buffers[0], *_AOtexture);
	}

	std::shared_ptr<PointRenderer> _pointRenderer{};
	std::shared_ptr<LineRenderer> _lineRenderer{};
	std::shared_ptr<FlatShadingPipeline> _pipeline{};
	std::shared_ptr<FlatShadingPipeline> _wireframePipeline{};
	glm::vec4 _color{};

	glm::mat4 _model = glm::identity<glm::mat4>();
	
	int _indexCount{};
	std::shared_ptr<RT::BufferAndMemory> _indexBuffer{};

	std::shared_ptr<RT::BufferAndMemory> _vertexBuffer{};

	std::shared_ptr<RT::GpuTexture> _texture{};
	std::shared_ptr<RT::GpuTexture> _AOtexture{};
	std::shared_ptr<RT::GpuTexture> _Perlintexture{};

	std::shared_ptr<RT::BufferGroup> _material{};
	
	RT::DescriptorSetGroup _perGeometryDescriptorSet;
};

std::shared_ptr<FlagMesh> GenerateFlag(
	std::shared_ptr<FlatShadingPipeline> const& pipeline,
	std::shared_ptr<FlatShadingPipeline> const& wireframePipeline
)
{
	std::vector<glm::vec3> vertices{};
	std::vector<std::tuple<int, int, int>> triangles{};   // Used to construct the growth tensors
	std::vector<glm::vec2> uvs{};
	std::vector<glm::vec3> normals{};

	Importer::ObjModel objModel{};
	bool success = Importer::LoadObj(Path::Instance->Get("models/chess_bishop/bishop.obj"), objModel);
	MFA_ASSERT(success == true);

	for (int i = 0; i < objModel.indices.size(); i += 3)
	{
		triangles.emplace_back(std::tuple{ objModel.indices[i], objModel.indices[i + 1], objModel.indices[i + 2] });
	}
	for (auto & vertex : objModel.vertices)
	{
		vertices.emplace_back(vertex.position);
		uvs.emplace_back(vertex.uv);
		normals.emplace_back(vertex.normal);
	}

	/*int xCount = 2;
	int yCount = 2;

	glm::vec3 start{ -1.5f, -1.0f, 3.0f };
	glm::vec3 end{ 1.5f, 1.0f, 3.0f };

	for (int j = 0; j < xCount; ++j)
	{
		float xT = static_cast<float>(j) / static_cast<float>(xCount - 1);
		for (int k = 0; k < yCount; ++k)
		{
			float yT = static_cast<float>(k) / static_cast<float>(yCount - 1);
			uvs.emplace_back(xT, 1.0f - yT);
			vertices.emplace_back(glm::vec3{
				glm::mix(start.x, end.x, xT),
				glm::mix(start.y, end.y, yT),
				end.z,
			});
		}
	}

	for (int j = 0; j < xCount - 1; ++j)
	{
		int currX = j * yCount;
		int nextX = currX + yCount;

		for (int i = 0; i < yCount - 1; ++i)
		{
			triangles.emplace_back(std::tuple{ currX + i, currX + i + 1, nextX + i + 1 });
			triangles.emplace_back(std::tuple{ currX + i,  nextX + i + 1, nextX + i});
		}
	}*/

	return std::make_shared<FlagMesh>(
		pipeline,
		wireframePipeline,
		glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f },
		vertices,
		triangles,
		uvs,
		normals
	);

}


int main(int argc, char* argv[])
{
	fileName = argv[1];
	fileNameAO = argv[2];
	MFA_LOG_DEBUG("Loading...");

	auto path = Path::Instantiate();

	LogicalDevice::InitParams params
	{
		.windowWidth = 1000,
		.windowHeight = 1000,
		.resizable = true,
		.fullScreen = false,
		.applicationName = "Flag app"
	};

	auto const device = LogicalDevice::Instantiate(params);
	assert(device->IsValid() == true);

	{
		auto swapChainResource = std::make_shared<SwapChainRenderResource>();
		auto depthResource = std::make_shared<DepthRenderResource>();
		auto msaaResource = std::make_shared<MSSAA_RenderResource>();
		auto displayRenderPass = std::make_shared<DisplayRenderPass>(
			swapChainResource,
			depthResource,
			msaaResource
			);

		auto const ui = std::make_shared<UI>(displayRenderPass);

		auto cameraBuffer = RB::CreateHostVisibleUniformBuffer(
			device->GetVkDevice(),
			device->GetPhysicalDevice(),
			sizeof(glm::mat4),
			device->GetMaxFramePerFlight()
		);

		//setting to arcball camera and setting position of camera
		ArcballCamera camera{};
		camera.Setposition({0.0f, -8.0f, 15.0f});

		auto ComputeViewProjectionMat4 = [&camera]()->glm::mat4
		{
			return camera.GetViewProjection();
		};

		auto cameraBufferTracker = std::make_shared<HostVisibleBufferTracker<glm::mat4>>(cameraBuffer, ComputeViewProjectionMat4());

		device->ResizeEventSignal2.Register([&cameraBufferTracker, &ComputeViewProjectionMat4]()->void
			{
				cameraBufferTracker->SetData(ComputeViewProjectionMat4());
			}
		);

		auto defaultSampler = RB::CreateSampler(LogicalDevice::Instance->GetVkDevice(), {});
		
		auto const pipeline = std::make_shared<FlatShadingPipeline>(
			displayRenderPass, 
			cameraBuffer, 
			defaultSampler,
			FlatShadingPipeline::Params {
				.maxSets = 100,
				.cullModeFlags = VK_CULL_MODE_NONE
			}
		);
		auto const wireframePipeline = std::make_shared<FlatShadingPipeline>(
			displayRenderPass,
			cameraBuffer,
			defaultSampler,
			FlatShadingPipeline::Params {
				.maxSets = 100,
				.cullModeFlags = VK_CULL_MODE_NONE,
				.polygonMode = VK_POLYGON_MODE_LINE
			}
		);
		
		auto flagMesh = GenerateFlag(pipeline, wireframePipeline);

		ui->UpdateSignal.Register(UI_Loop);

		static const uint32_t FixedDeltaTimeMs = 1000 / 120;
		static const float PBD_FixedDeltaTimeSec = static_cast<float>(FixedDeltaTimeMs) / 1000.0f;
		SDL_GL_SetSwapInterval(0);
		SDL_Event e;
		uint32_t deltaTimeMs = FixedDeltaTimeMs;
		float deltaTimeSec = PBD_FixedDeltaTimeSec;
		uint32_t startTime = SDL_GetTicks();
		
		bool shouldQuit = false;

		while (shouldQuit == false)
		{
			//Handle events
			while (SDL_PollEvent(&e) != 0)
			{
				//User requests quit
				if (e.type == SDL_QUIT)
				{
					shouldQuit = true;
				}
			}

			device->Update();

			camera.Update(deltaTimeSec);
			if (camera.IsDirty())
			{
				cameraBufferTracker->SetData(camera.GetViewProjection());
			}

			ui->Update();

			auto recordState = device->AcquireRecordState(swapChainResource->GetSwapChainImages().swapChain);
			if (recordState.isValid == true)
			{
				device->BeginCommandBuffer(
					recordState,
					RT::CommandBufferType::Compute
				);
				device->EndCommandBuffer(recordState);

				device->BeginCommandBuffer(
					recordState,
					RT::CommandBufferType::Graphic
				);

				cameraBufferTracker->Update(recordState);

				displayRenderPass->Begin(recordState);

				flagMesh->Render(recordState);
				ui->Render(recordState, deltaTimeSec);

				displayRenderPass->End(recordState);
				device->EndCommandBuffer(recordState);

				device->SubmitQueues(recordState);
				device->Present(recordState, swapChainResource->GetSwapChainImages().swapChain);
			}

			deltaTimeMs = SDL_GetTicks() - startTime;
			if (FixedDeltaTimeMs > deltaTimeMs)
			{
				SDL_Delay(FixedDeltaTimeMs - deltaTimeMs);
			}
			deltaTimeMs = SDL_GetTicks() - startTime;
			deltaTimeSec = static_cast<float>(deltaTimeMs) / 1000.0f;
			startTime = SDL_GetTicks();
		}

		device->DeviceWaitIdle();
	
	}

	return 0;
}