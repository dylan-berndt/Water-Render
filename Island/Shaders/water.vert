#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 aTangent;

out vec3 FragPos;

uniform int vertSamples = 16;
uniform float dragMult = 0.2;

vec2 wavedx(vec2 position, vec2 direction, float frequency, float timeshift) {
    float x = dot(direction, position) * frequency + timeshift;
    float wave = exp(sin(x) - 1.0);
    float dx = wave * cos(x);
    return vec2(wave, -dx);
}

float getWaves(vec2 position) {
    float iter = 0.0;
    float frequency = 1.0;
    float timeMultiplier = 2.0;
    float weight = 1.0;
    float sumOfValues = 0.0;
    float sumOfWeights = 0.0;
    for(int i = 0; i < vertSamples; i++) {
        vec2 p = vec2(sin(iter), cos(iter));
        vec2 res = wavedx(position, p, frequency, time * timeMultiplier);

        position += p * res.y * weight * dragMult;

        sumOfValues += res.x * weight;
        sumOfWeights += weight;

        weight *= 0.82;
        frequency *= 1.18;
        timeMultiplier *= 1.07;

        iter += 1232.399963;
    }
    return 4.0 * sumOfValues / sumOfWeights;
}

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);

    vec4 shift = vec4(0.0, getWaves(worldPos.xz), 0.0, 0.0);
    gl_Position = projection * view * model * (vec4(aPos, 1.0) + shift * 0.6);
}
