#include <vulkan/vulkan.h>

void lineInitGeometryAndBuffers();
void lineUpdateGeometryAndBuffers();
void lineDestroyBuffers();
void lineDraw();

extern int n;

VkBuffer lineGetVerticesBuffer();