#pragma once

#include <vulkan/vulkan.h>

namespace MyApp
{
    class LinePipeline
    {
    public:

        explicit LinePipeline();

        ~LinePipeline();

        [[nodiscard]]
        VkPipeline GetPipeline() const
        {
            return _pipeline;
        }

    private:

        VkPipeline _pipeline{};

    };
}
