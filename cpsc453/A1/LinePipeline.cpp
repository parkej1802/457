#include "LinePipeline.hpp"

#include "VulkanLaunchpad.h"

MyApp::LinePipeline::LinePipeline()
{
		_pipeline = vklCreateGraphicsPipeline(
				VklGraphicsPipelineConfig{
                      // Vertex Shader from memory:
                      "#version 450\n"
                      "layout(location = 0) in vec3 position;\n"
                      "void main() {\n"
                      "    gl_Position = vec4(position.x, -position.y, position.z, 1);\n"
                      "}\n",
                      // Fragment shader from memory:
                      "#version 450\n"
                      "layout(location = 0) out vec4 color; \n"
                      "void main() {  \n"
                      "    color = vec4(1, 0, 0, 1); \n"
                      "}\n",
                      // Further config parameters:
                      {
                          VkVertexInputBindingDescription { 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX }
                      },
                      {
                          VkVertexInputAttributeDescription { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0u }
                      },
                      VK_POLYGON_MODE_FILL,
                      VK_CULL_MODE_NONE,
                      { /* no descriptors */ }
				}, 
				/* load shaders from memory: */ true, 
				PrimitiveTopology::eLineStrip
		);
}

MyApp::LinePipeline::~LinePipeline()
{
	vklDestroyGraphicsPipeline(_pipeline);
}
