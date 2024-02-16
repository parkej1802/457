I'm following the same build procedure as the starter code. 

Once we have created build then we put in terminal

cd HW4
./HW

left and right keys to rotate

I have done the bonus for shading and the angle of earth and the moon 
almost every sphere worked on this format

    // Earth Sphere
    //we can calculate the discriminant and if it intersects with the ray then we can calculate the intersection min and the max. 
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

	if sphere is behind then it shouldn't be raytraced so it doesn't show
	
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
                
                // this is bonus part for shading

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

                // change texture by time * speed
                // normalize coordinates for texture sampling. 
                // Top-left of texture is (0,0) in Vulkan, so we can stick to spherical coordinates
                color = texture(textures[2], 
            vec2(1.0+0.5*(theta + pc.time * PI)/PI, phi/PI )) * col1;
            }
        }
        //behindEarth = tmax;
        behindMoon = tmax;

    }



for bonus angle adjustment

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

i set the angle to 5.14 degree and set that value to the y axis


float rotationAngle1 = 23.44 *PI/180;
float rotationAngle2 = -6.68 * PI/180;

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

    ipoint = ipoint - centerEarth;
    ipoint =  rotationMatrix1 * ipoint.xyz;
    ipoint = rotationMatrix2 * ipoint.xyz;
    ipoint = ipoint + centerEarth;

set angle for earth 

    ipoint = ipoint - centerMoon;
    ipoint =  rotationMatrix1 * ipoint.xyz;
    ipoint =  rotationMatrix3 * ipoint.xyz;
    ipoint = ipoint + centerMoon;

this is for moon

