#version 310 es

precision highp image2D;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//Texture to store the data in since pi doesnt support SSBO's in vert >:(
layout(r32f, binding = 0) uniform image2D particleDataTexture;

// variables
uniform float deltaTime;
uniform vec3 gravity;
uniform vec3 startPosition;
uniform vec3 startPosSize;
uniform int maxParticles;  
uniform int particlesPerFrame;
uniform vec3 spawnDirection;
uniform float spawnDirectionRandomness;
uniform vec2 spawnVelocity;
uniform vec2 particleLifeTime;
uniform int seed;

//Atomic counter to spawn particles in a controlled way
layout(binding = 1, offset = 0) uniform atomic_uint spawnIndex;

uint state = 0u;

uint xorshift()
{
	uint x = state;
	x ^= x << 13; // Shift left 13
	x ^= x >> 17; // Shift right 17
	x ^= x << 5;  // Shift left 5
	state = x;   // Update the state
	return x;
}

// Function to generate a random float between [0, 1]
float rand()
{
	return float(xorshift()) * 2.3283064365387e-10;
}

uint randInt(uint range)
{
	return xorshift() % range;
}

uint pcg_hash(uint _seed)
{
    uint state = _seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}


//HOW THE DATA IS STORED IN THE texture
/*
y coords  value
0         posX
1         posY
2         posZ
3         velX
4         velY
5         velZ
6         life
7         lifetime
*/

void main() {
    uint id = gl_GlobalInvocationID.x;  // Particle index

    ivec2 texCoords = ivec2(id, 0);  //1D to 2D mapping for texture

    float life = imageLoad(particleDataTexture, texCoords + ivec2(0,6)).r - deltaTime;
    //Check if the particle needs respawning
    if (life <= 0.0) {
        uint spawnIdx = atomicCounterIncrement(spawnIndex);
        if (spawnIdx < uint(particlesPerFrame)) {
            state = pcg_hash(uint(seed) * id);

			//Reset particle
			vec3 startOffset = normalize(vec3(rand() - 0.5, rand() - 0.5, rand() - 0.5)) * startPosSize;
			vec3 position = startPosition + startOffset;
			float force = mix(spawnVelocity.x,spawnVelocity.y, rand());
			vec3 dir = normalize(spawnDirection + vec3(rand() - 0.5, rand() - 0.5, rand() - 0.5)*spawnDirectionRandomness);
            vec3 velocity = dir * force;
			life = rand() * (particleLifeTime.y-particleLifeTime.x)+particleLifeTime.x;

            //store in texture
			imageStore(particleDataTexture, texCoords + ivec2(0, 0), vec4(position.x,0,0,0));
			imageStore(particleDataTexture, texCoords + ivec2(0, 1), vec4(position.y,0,0,0));
			imageStore(particleDataTexture, texCoords + ivec2(0, 2), vec4(position.z,0,0,0));
			imageStore(particleDataTexture, texCoords + ivec2(0, 3), vec4(velocity.x,0,0,0));
			imageStore(particleDataTexture, texCoords + ivec2(0, 4), vec4(velocity.y,0,0,0));
			imageStore(particleDataTexture, texCoords + ivec2(0, 5), vec4(velocity.z,0,0,0));
			imageStore(particleDataTexture, texCoords + ivec2(0, 6), vec4(life,0,0,0));
			imageStore(particleDataTexture, texCoords + ivec2(0, 7), vec4(life,0,0,0));
			return;
		}
    }

        // Update velocity and position
        vec3 velocity;
		velocity.x = imageLoad(particleDataTexture, texCoords + ivec2(0,3)).r;
		velocity.y = imageLoad(particleDataTexture, texCoords + ivec2(0,4)).r;
		velocity.z = imageLoad(particleDataTexture, texCoords + ivec2(0,5)).r;

        velocity += gravity * deltaTime;

		vec3 pos;
		pos.x = imageLoad(particleDataTexture, texCoords + ivec2(0,0)).r;
		pos.y = imageLoad(particleDataTexture, texCoords + ivec2(0,1)).r;
		pos.z = imageLoad(particleDataTexture, texCoords + ivec2(0,2)).r;
        pos += velocity * deltaTime;

        //store in texture
		imageStore(particleDataTexture, texCoords + ivec2(0, 0), vec4(pos.x,0,0,0));
		imageStore(particleDataTexture, texCoords + ivec2(0, 1), vec4(pos.y,0,0,0));
		imageStore(particleDataTexture, texCoords + ivec2(0, 2), vec4(pos.z,0,0,0));
		imageStore(particleDataTexture, texCoords + ivec2(0, 3), vec4(velocity.x,0,0,0));
		imageStore(particleDataTexture, texCoords + ivec2(0, 4), vec4(velocity.y,0,0,0));
		imageStore(particleDataTexture, texCoords + ivec2(0, 5), vec4(velocity.z,0,0,0));
		imageStore(particleDataTexture, texCoords + ivec2(0, 6), vec4(life,0,0,0));
		float lifeTime = imageLoad(particleDataTexture, texCoords + ivec2(0,7)).r;
		imageStore(particleDataTexture, texCoords + ivec2(0, 7), vec4(lifeTime,0,0,0));
}