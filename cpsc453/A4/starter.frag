#version 450

#define PI 3.1415926535897932384626433832795
#define MAX_TEXTURES 4		// save some space in the push constants by hard-wiring this

layout(location = 0) out vec4 color;

// interpolated position and direction of ray in world space
layout(location = 0) in vec3 p;
layout(location = 1) in vec3 d;

//push constants block
layout( push_constant ) uniform constants
{
	mat4 invView; // camera-to-world
	vec4 proj; // (near, far, aspect, fov)
	float time;
} pc;

layout(binding = 0) uniform sampler2D textures[ MAX_TEXTURES ];

// Material properties
vec3 bg_color = vec3(0.00,0.00,0.05);

float ambient_strength = 0.1;
vec3 specular_strength = vec3(1.0, 1.0, 1.0);
float specular_power = 76.8;

vec3 lightPos = vec3(0.0, 0.0, 0.0);
vec3 lightCol = vec3(1.0, 1.0, 1.0);

vec3 col = vec3(1.0,1.0,1.0);


vec4 col1;

float earthO = 365;
float moonO = 27;
float sun = 27;

vec3 centerSpace = vec3(0.0f,0.0f,0.0f);
vec3 centerSun = vec3(0.0f,0.0f,0.0f);
vec3 centerEarth = vec3(20.0f,0.0f,0.0f);
vec3 centerMoon = vec3(25.0f,0.0f,0.0f);


float rotationAngle = -90 * PI/180; 
float rotationAngle1 = 23.44 *PI/180;
float rotationAngle2 = -6.68 * PI/180;
float rotationAngle3 = 5.14 * PI/180;

mat3 rotationMatrix = mat3(
    cos(rotationAngle3), 0, sin(rotationAngle3),
    0, 1, 0,
    -sin(rotationAngle3), 0, cos(rotationAngle3)
);

mat3 rotationMatrix1 = mat3(
    1.0, 0, 0, 
    0, cos(rotationAngle), -sin(rotationAngle),
    0, sin(rotationAngle), cos(rotationAngle)
);

mat3 rotationMatrix2 = mat3(
    1.0, 0, 0, 
    0, cos(rotationAngle1), -sin(rotationAngle1),
    0, sin(rotationAngle1), cos(rotationAngle1)
);

mat3 rotationMatrix3 = mat3(
    1.0, 0, 0, 
    0, cos(rotationAngle2), -sin(rotationAngle2),
    0, sin(rotationAngle2), cos(rotationAngle2)
);




float angle = 5.14 * PI/180; // rotation angle in degrees


float time = 0.0f;

float speed = 1.0f; 

// orbit rotation function 
void orbitRotation(float time) {
    // calculate speed
    float earthOrbitSpeed = speed / 365.0f; 
    float moonOrbitSpeed = speed / 27.0f; 

    // calculate angle
    float earthAngle = (PI) * earthOrbitSpeed * time;
    float moonAngle = (PI) * moonOrbitSpeed * time;

    // set the position of earth and moon 
    centerEarth = vec3(cos(earthAngle) * 2, 0.0, sin(earthAngle) * 2);
    centerMoon = centerEarth + vec3(cos(moonAngle) * 0.5, 0.5 * angle, sin(moonAngle) * 0.5);
}


void main() {
    
    // do rotation by time
    orbitRotation(pc.time);

    // get radius of spheres
    float radiusSpace = (1000.0f / 100.0f);
    float radiusSun = (50.0f / 100.0f);
    float radiusEarth = (20.0f / 100.0f);
    float radiusMoon = (5.0f / 100.0f);
    
    // get ray direction
    vec3 dir = normalize(d);

    // get B and C for each sphere for discriminant calculation
    vec3 centerS = p - centerSpace;
    vec3 centerSu = p - centerSun;
    vec3 centerE = p - centerEarth;
    vec3 centerM = p - centerMoon;


    float spaceB = dot(2 * dir, centerS);
    float spaceC = dot((p - centerSpace),(p - centerSpace)) - (radiusSpace * radiusSpace);

    float discriminant = spaceB*spaceB - 4.0*(spaceC);

    float sunB = dot(2 * dir, centerSu);
    float sunC = dot((centerSu),(centerSu)) - (radiusSun * radiusSun);

    float discriminant1 = sunB*sunB -4.0*(sunC);

    float earthB = dot(2 * dir, centerE);
    float earthC = dot((p - centerEarth),(p - centerEarth)) - (radiusEarth * radiusEarth);

    float discriminant2 = earthB*earthB -4.0*(earthC);

    float moonB = dot(2 * dir, centerM);
    float moonC = dot((p - centerMoon), (p- centerMoon)) - (radiusMoon * radiusMoon);

    float discriminant3 = moonB*moonB -4.0*(moonC);

    color = vec4(bg_color, 1.0);

    float behind = 1000.0f;
    float behindEarth = 1000.0f;
    float behindMoon = 1000.f;

    vec3 N = normalize(d);
    vec3 L = normalize(lightPos);
    vec3 V = normalize(-p);
    vec3 R = reflect(L, N);

    vec3 diffuse = max(dot(N, L), 0.0) * col;
    vec3 ambient = ambient_strength * col;
    vec3 specular = pow(max(dot(R,V), 0.0), specular_power) * specular_strength;
    col1 = vec4((ambient + diffuse + specular)*lightCol, 1.0);

    // Universe sphere

    //if discriminant is less than 0 then it doesn't intersection, larger than 0 will be intersection, so we can get 2 intersection point 

    if( discriminant >= 0.0) {
        // determine intersection point
        float t1 = 0.5 * (-spaceB - sqrt(discriminant));
        float t2 = 0.5 * (-spaceB + sqrt(discriminant));
        float tmin, tmax;
        float t;
        if(t1 < t2) {
            tmin = t1;
            tmax = t2;
        } else {
            tmin = t2;
            tmax = t1;
        }
        //calculate the intersection point and if it's larger than behind value it will do the ray tracing.
        // this let behind spheres doesn't show when other spheres are infront of another sphere
        
        if (behind > tmax) {
            if(tmax > 0.0) {
                t = (tmin > 0) ? tmin : tmax;
                vec3 ipoint = p + t*(dir);
                vec3 normal = normalize(ipoint);
                
                // determine texture coordinates in spherical coordinates
                
                // First rotate about x through 90 degrees so that y is up.


                ipoint.z = -ipoint.z;
                ipoint = ipoint.xzy;

                float phi = acos((ipoint.z - centerSpace.z) / radiusSpace);
                float theta;

                theta = atan((ipoint.y - centerSpace.y) / (ipoint.x - centerSpace.x));

                // normalize coordinates for texture sampling. 
                // Top-left of texture is (0,0) in Vulkan, so we can stick to spherical coordinates
                color = texture(textures[0], 
            vec2(1.0 +0.5* theta /PI, phi/PI));
            }
            
        }
        behind = tmax;
    }

    // Sun sphere
    if( discriminant1 >= 0.0) {
        // determine intersection point
        float t1 = 0.5 * (-sunB - sqrt(discriminant1));
        float t2 = 0.5 * (-sunB + sqrt(discriminant1));
        float tmin, tmax;

        float t;
        if(t1 < t2) {
            tmin = t1;
            tmax = t2;
        } else {
            tmin = t2;
            tmax = t1;
        }


        if (behind > tmax) {
            if(tmax > 0.0) {
                t = (tmin > 0) ? tmin : tmax;
                vec3 ipoint = p + t*(dir);
                vec3 normal = normalize(ipoint);
                
                // determine texture coordinates in spherical coordinates
                
                // First rotate about x through 90 degrees so that y is up.

                ipoint = ipoint - centerSun;
                ipoint =  rotationMatrix1 * ipoint.xyz;
                ipoint = ipoint + centerSun;

                float phi = acos((ipoint.z - centerSun.z)/ radiusSun);
                float theta;

                if (ipoint.x - centerSun.x == 0.0) {
                    theta = sign(ipoint.y - centerSun.y) * PI / 3.0;
                } else {
                    theta = atan((ipoint.y - centerSun.y) / (ipoint.x - centerSun.x));
                }

                if (ipoint.x - centerSun.x < 0.0) {
                    theta += PI;
                }

                if (theta < 0.0) {
                    theta += 2.0 * PI;
                }

                
                // normalize coordinates for texture sampling. 
                // Top-left of texture is (0,0) in Vulkan, so we can stick to spherical coordinates
                
                color = texture(textures[1], 
                vec2(1 + 0.5*(theta + pc.time * PI / 27)/PI, phi/PI));
            }
        }
        behindEarth = tmax;
        behindMoon = tmax;
    }

    // Earth Sphere
    if( discriminant2 >= 0.0) {
        // determine intersection point
        float t1 = 0.5 * (-earthB - sqrt(discriminant2));
        float t2 = 0.5 * (-earthB + sqrt(discriminant2));
        float tmin, tmax;
        float t;
        float ti = 0.0f;

        if(t1 < t2) {
            tmin = t1;
            tmax = t2;
        } else {
            tmin = t2;
            tmax = t1;
        }

        if (behindMoon > tmax) {
            if(tmax > 0.0) {
                t = (tmin > 0) ? tmin : tmax;
                vec3 ipoint = p + t*(dir);
                vec3 normal = normalize(ipoint); 
                
                // determine texture coordinates in spherical coordinates
                
                // First rotate about x through 90 degrees so that y is up.
                //normal.z = -normal.z;
                normal = normal.xzy;

                vec3 ray_dir = normalize(centerSun - ipoint);

                // get t min of moon and the earth if moon is infront of earth then it draws shadow otherwise not draws

                float moonB = dot(2 * ray_dir, (centerSun - centerMoon));
                float moonC = dot((centerSun - centerMoon),(centerSun - centerMoon)) - (radiusMoon * radiusMoon);

                float earthBS = dot(2* ray_dir, centerSun - centerEarth);
                float earthCS = dot((centerSun -centerEarth),(centerSun - centerEarth)) -(radiusEarth * radiusEarth);

                float discriminant = moonB*moonB -4.0*(moonC);
                float discriminantES = earthBS * earthBS - 4.0 *(earthCS);
                float shadow = 1;

                if (discriminant >= 0.0) {
                    
                    float t1 = 0.5 * (-moonB - sqrt(discriminant));
                    float t2 = 0.5 * (-moonB + sqrt(discriminant));

                    float tmin, tmax;

                    if(t1 < t2) {
                        tmin = t1;
                        tmax = t2;
                    } else {
                        tmin = t2;
                        tmax = t1;
                    }

                    float t1E = 0.5 * (-earthBS - sqrt(discriminantES));
                    float t2E = 0.5 * (-earthBS + sqrt(discriminantES));

                    float tmin1, tmax1;


                    if(t1E < t2E) {
                        tmin1 = t1E;
                        tmax1 = t2E;
                    } else {
                        tmin1 = t2E;
                        tmax1 = t1E;
                    }

                    if (tmin > tmin1) {
                        shadow = 0.0f;
                    }else{
                        shadow = 1.0f;
                    }

                }
                else {
                    shadow = 1.0f;
                }


                // get lightDir from center to the earth and do the phong shading based on that point
                vec3 lightDir = lightPos - ipoint;

            
                vec3 N = normalize(ipoint - centerEarth);
                vec3 L = normalize(lightDir);
                vec3 V = normalize(d);
                vec3 R = reflect(L, N);

                vec3 diffuse = max(dot(N, L), 0.0) * col;
                vec3 ambient = ambient_strength * col;
                vec3 specular = pow(max(dot(R,V), 0.0), specular_power) * specular_strength;
                col1 = vec4((ambient + (diffuse + specular)*shadow)*lightCol, 1.0);


                // rotation of earth 90 degree first and then do 23.44 degree 
                ipoint = ipoint - centerEarth;
                ipoint =  rotationMatrix1 * ipoint.xyz;
                ipoint = rotationMatrix2 * ipoint.xyz;
                ipoint = ipoint + centerEarth;

                float phi = acos((ipoint.z - centerEarth.z)/radiusEarth);
                float theta;
                if (ipoint.x - centerEarth.x == 0.0) {
                    //theta = sign(ipoint.y - centerEarth.y) * PI / 2.0;
                } else {
                    theta = atan((ipoint.y - centerEarth.y) / (ipoint.x - centerEarth.x));
                }

                if (ipoint.x - centerEarth.x < 0.0) {
                    theta += PI;
                }

                if (theta < 0.0) {
                    theta += 2.0 * PI;
                }

                // change texture by time
                // normalize coordinates for texture sampling. 
                // Top-left of texture is (0,0) in Vulkan, so we can stick to spherical coordinates
                color = texture(textures[2], 
            vec2(1.0+0.5*(theta + pc.time * PI)/PI, phi/PI )) * col1;
            }
        }
        //behindEarth = tmax;
        behindMoon = tmax;

    }

    // Moon sphere
    if( discriminant3 >= 0.0) {
        // determine intersection point
        float t1 = 0.5 * (-moonB - sqrt(discriminant3));
        float t2 = 0.5 * (-moonB + sqrt(discriminant3));
        float tmin, tmax;
        float t;
        if(t1 < t2) {
            tmin = t1;
            tmax = t2;
        } else {
            tmin = t2;
            tmax = t1;
        }

        // calculate behind tmax value so if moon is behind each sphere then it doesn't render
        if (behindMoon > tmax) {
            if (behindEarth > tmax) {

            
            if(tmax > 0.0) {
                t = (tmin > 0) ? tmin : tmax;
                vec3 ipoint = p + t*(dir);
                vec3 normal = normalize(ipoint);
                
                // determine texture coordinates in spherical coordinates
                
                // First rotate about x through 90 degrees so that y is up.


                // same process as earth, for this one we have to draw shadow when moon is behind of earth 
                vec3 ray_dir = normalize(centerSun - ipoint);

                float moonB = dot(2 * ray_dir, (centerSun - centerMoon));
                float moonC = dot((centerSun - centerMoon),(centerSun - centerMoon)) - (radiusMoon * radiusMoon);

                float earthBS = dot(2* ray_dir, centerSun - centerEarth);
                float earthCS = dot((centerSun -centerEarth),(centerSun - centerEarth)) -(radiusEarth * radiusEarth);

                float discriminant = moonB*moonB -4.0*(moonC);
                float discriminantES = earthBS * earthBS - 4.0 *(earthCS);
                float shadow = 1;

                if (discriminantES >= 0.0) {
                    
                    float t1 = 0.5 * (-moonB - sqrt(discriminant));
                    float t2 = 0.5 * (-moonB + sqrt(discriminant));

                    float tmin1, tmax1;

                    if(t1 < t2) {
                        tmin1 = t1;
                        tmax1 = t2;
                    } else {
                        tmin1 = t2;
                        tmax1 = t1;
                    }

                    float t1E = 0.5 * (-earthBS - sqrt(discriminantES));
                    float t2E = 0.5 * (-earthBS + sqrt(discriminantES));

                    float tmin2, tmax2;


                    if(t1E < t2E) {
                        tmin2 = t1E;
                        tmax2 = t2E;
                    } else {
                        tmin2 = t2E;
                        tmax2 = t1E;
                    }

                    if (tmin1 > tmin2) {
                        shadow = 1.0f;
                    }else{
                        shadow = 0.0f;
                    }

                }
                else {
                    shadow = 1.0f;
                }

                vec3 lightDir = lightPos - ipoint;

                vec3 N = normalize(ipoint - centerMoon);
                vec3 L = normalize(lightDir);
                vec3 V = normalize(-p);
                vec3 R = reflect(L, N);

                vec3 diffuse = max(dot(N, L), 0.0) * col;
                vec3 ambient = ambient_strength * col;
                vec3 specular = pow(max(dot(R,V), 0.0), specular_power) * specular_strength;
                col1 = vec4((ambient + (diffuse + specular)*shadow)*lightCol, 1.0);

                ipoint = ipoint - centerMoon;
                ipoint =  rotationMatrix1 * ipoint.xyz;
                ipoint =  rotationMatrix3 * ipoint.xyz;
                ipoint = ipoint + centerMoon;

                float phi = acos((ipoint.z - centerMoon.z)/radiusMoon);
                float theta;
                
                if (ipoint.x - centerMoon.x == 0.0) {
                    theta = sign(ipoint.y - centerMoon.y) * PI / 2.0;
                } else {
                    theta = atan((ipoint.y - centerMoon.y) / (ipoint.x - centerMoon.x));
                }

                if (ipoint.x - centerMoon.x < 0.0) {
                    theta += PI;
                }

                if (theta < 0.0) {
                    theta += 2.0 * PI;
                }

                // normalize coordinates for texture sampling. 
                // Top-left of texture is (0,0) in Vulkan, so we can stick to spherical coordinates
                color = texture(textures[3], 
            vec2(1.0+0.5*(theta + pc.time * PI/27)/PI, phi/PI )) * col1;

            }
            }
            behindEarth = tmax;
        }
        behindMoon = tmax;
    }
}
