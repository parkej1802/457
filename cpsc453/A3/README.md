To run this file go to

https://github.com/MohammadFakhreddin/MFA_EngineTemplate/tree/main

In terminal
 1   git clone https://github.com/MohammadFakhreddin/MFA_EngineTemplate/tree/main
 
 2   cd MFA_EngineTemplate

 3   git clone https://github.com/Microsoft/vcpkg.git
 
 4   cd vcpkg

 5  .\vcpkg\bootstrap-vcpkg.bat
 
 6  ./vcpkg install sdl2[vulkan] --triplet x64-windows
    (if linux change "x64-windows" to "x64-linux")
    
 7  mkdir build
 8  cd build
 9  cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
 
 10  make
 11 cd executables
 12 cd texture-mapping
 13 ./TextureMapping [path to texture png file] [path to AO png file]
 
 I set path to the file is in models/chess_bishop/[fileName] for both texture and AO so in order to use different texture from outside, user need to move that png file into "models/chess_bishop" folder to use that png file. 

I moved Flag_of_Canada.png into chess_bishop folder to users to input easier. 

./TextureMapping Flag_of_Canada.png bishop.ao.png 

if there is no Flag_of_Canada.png then below to compile

./TextureMapping bishop.colour.black.png bishop.ao.png 

To compile shader code

npm run compile-shaders

if we get "sh: line 1: glslc: command not found" this error then run

source /home/share/gfx/vulkanSDK/1.3.261.1/setup-env.sh

and "run npm run compile-shaders" again


==========================================================================================
I've started with perlin noise function, 
        for (int i = 0; i <= 4; i++) {
                    colorPerlin += (baseColorTexture.Sample(textureSampler, float2(input.baseColorUV * pow(2, i)))/ pow(2,i));
         }
        colorPerlin = 0.5f * (1.0f + sin(pushConstants.m * PI * (input.baseColorUV.x + input.baseColorUV.y + colorPerlin)));
I have used formula in assignment description for this code 

and then I started to work on changing m part in perlin noise
	ImGui::Checkbox("Perlin Noise enable", &perlinNoise);
	ImGui::SliderInt("Control m", &m, 24, 48);
these keys show gui which can control to enable or change int of perlin noise
in order to pass over the boolean value and int value over flatshading.frag file I had to use pushconstants to send to frag and also set it in hpp file to pass over
  struct PushConstants
        {
            glm::mat4 model;
            int perlinNoiseInt;
            int m;
            int AOTextureInt;
            int textureInt;
        };
        
Now I was able to control the values. Also since pushconstants only withs with int value but enable part works with boolean so I used another int value of 0 to 1 to send int value that can be shown in boolean value. 

RT::DescriptorSetGroup FlatShadingPipeline::CreatePerGeometryDescriptorSetGroup

in this line, I have added 
		RT::GpuTexture const & texture,
		RT::GpuTexture const & AOtexture,
		RT::GpuTexture const & Perlintexture 
variables to work with different textures, since we need to add texture on top of texture so we must have each variables to make that work.

so I have created each texture info

		VkDescriptorImageInfo const texturesSamplerInfo1{
			.sampler = VK_NULL_HANDLE,
			.imageView = AOtexture.imageView->imageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		VkDescriptorImageInfo const texturesSamplerInfo2{
			.sampler = VK_NULL_HANDLE,
			.imageView = Perlintexture.imageView->imageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		descriptorSetSchema.AddImage(&texturesSamplerInfo1, 1);
		descriptorSetSchema.AddImage(&texturesSamplerInfo2, 1);
and addImage so that passes over to fragment like below so we can now use each texture. 
        Texture2D baseColorTexture : register(t3, space1);
        Texture2D AOColorTexture : register(t2, space1);
        Texture2D textureColorTexture : register(t1, space1);

Now use baseColorTexture to work with perlin, when we get work with each texture but we have to show it in 1 color since it only passes color variable that shows the texture. Therefore, we have to multiply the color with perlin texture or AO texture when perlin or AO textures are enabled. Then those enabled texture color will be combined.

I've started to work on specular part, I used function in HW3 shader part, In the vert file
          float4 worldPosition = mul(pushConsts.model, float4(input.position, 1.0));
        output.position = mul(vpBuff.viewProjection, worldPosition);
        output.baseColorUV = input.baseColorUV;
        output.worldNormal = normalize(mul((float3x3)pushConsts.model, input.normal));
        //output.viewDir = normalize(float3(0.0f, 0.0f, 0.0f) - worldPosition.xyz);
        float3 cameraPosition = float3(vpBuff.viewProjection._41, vpBuff.viewProjection._42,            vpBuff.viewProjection._43);
        output.viewDir = normalize(cameraPosition - worldPosition.xyz);

I set the viewDir. and send that to frag file, I used in  
        float3 V = normalize(input.viewDir); 
Now I had most of all the variables to do the specular in frag file so I've done 
        float3 color2 = ambient * color + dirLight + specular ; 

Once those were done, I've started to work on rotation,
in the render function in textureMapping
        glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), x, glm::vec3{1.0f, 0.0f, 0.0f});
		glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), y, glm::vec3{0.0f, 1.0f, 0.0f});
		glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), z, glm::vec3{0.0f, 0.0f, 1.0f});
		glm::mat4 rotateModel = rotateX * rotateY * rotateZ;
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

bascially I had to create rotation function and send that over to the model. 
Then worked on command line argument, there is main function in TextureMappinp.cpp
I needed to add argc argv in main function and get the argv as fileName and use that value to path.
	    fileName = argv[1];
	    fileNameAO = argv[2];
	
	first argument will be texture png file and second will be AOtexture png file
	
I had to change camera to arcball so changed observerCamera to ArcballCamera. Now I can scale up or down with mouse scroll.


