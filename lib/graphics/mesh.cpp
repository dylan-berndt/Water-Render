
#include "mesh.h"

using namespace std;

int Mesh::totalVertices = 0;

void Material::use(ShaderProgram &shader) {
    for (auto pair : booleans) {
        shader.assign(pair.first, pair.second);
    }
    for (auto pair : integers) {
        shader.assign(pair.first, pair.second);
    }
    for (auto pair : floats) {
        shader.assign(pair.first, pair.second);
    }
    for (auto pair : vec4s) {
        shader.assign(pair.first, pair.second);
    }
    for (auto pair : vec3s) {
        shader.assign(pair.first, pair.second);
    }
    for (auto pair : vec2s) {
        shader.assign(pair.first, pair.second);
    }
    for (auto pair : mat4s) {
        shader.assign(pair.first, pair.second);
    }

    int num = 1;
    if (!textures.count("baseTexture")) {
        shader.assign("baseTexture.valid", 0);
        glActiveTexture(GL_TEXTURE0 + num);
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.assign("baseTexture.id", num);
        num++;
    }
    if (!textures.count("bumpTexture")) {
        shader.assign("bumpTexture.valid", 0);
        glActiveTexture(GL_TEXTURE0 + num);
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.assign("bumpTexture.id", num);
        num++;
    }

    for (auto pair : textures) {
        pair.second.activate(GL_TEXTURE0 + num);
        shader.assign(pair.first + ".valid", 1);
        shader.assign(pair.first + ".id", num);
        shader.assign(pair.first + ".offset", pair.second.offset);
        shader.assign(pair.first + ".scale", pair.second.scale);
        shader.assign(pair.first + ".bumpMultiplier", pair.second.bumpMultiplier);
        num++;
    }

    for (auto pair : textureArrays) {
        pair.second.activate(GL_TEXTURE0 + num);
        shader.assign(pair.first, num);
        num++;
    }

    for (auto pair : cubeMaps) {
        glActiveTexture(GL_TEXTURE0 + num);
        pair.second.bind();
        shader.assign(pair.first, num);
        num++;
    }

    glActiveTexture(GL_TEXTURE0 + num);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ShaderProgram::shadowMap);
    shader.assign("shadowMap", num);
}

void Mesh::resetMesh(vector<Vertex> v, vector<int> i) {
    vertices = v;
    indices = i;

    model = glm::mat4(1.0);

    setupMesh();
}

Mesh::Mesh(vector<Vertex> v, vector<int> i) {
    vertices = v;
    indices = i;

    model = glm::mat4(1.0);

    setupMesh();
}

void Mesh::draw(ShaderProgram &shader) {
    material.use(shader);

    shader.assign("model", model);

    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void Mesh::setupMesh() {
    totalVertices += vertices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    calculateTangents();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    glBindVertexArray(0);
}

void Mesh::calculateTangents() {
    for (int t = 0; t < indices.size(); t += 3) {
        Vertex v1 = vertices[indices[(t)]];
        Vertex v2 = vertices[indices[(t + 1)]];
        Vertex v3 = vertices[indices[(t + 2)]];

        glm::vec3 q1 = v2.position - v1.position;
        glm::vec3 q2 = v3.position - v1.position;

        glm::vec2 u1 = v2.texCoord - v1.texCoord;
        glm::vec2 u2 = v3.texCoord - v1.texCoord;

        float r = 1.0f / (u1.x * u2.y - u2.x * u1.y);

        glm::vec3 tangent   = (q1 * u2.y - q2 * u1.y) * r;
        glm::vec3 bitangent = (q2 * u1.x - q1 * u2.x) * r;

        for (int i = 0; i < 3; i++) {
            Vertex current = vertices[indices[t + i]];
            glm::vec3 currentTangent = glm::normalize(tangent - current.normal * glm::dot(tangent, current.normal));

            if (glm::dot(glm::cross(current.normal, currentTangent), bitangent) < 0.0f){
                currentTangent = currentTangent * -1.0f;
            }

            current.tangent = currentTangent;
        }
    }
}

Mesh flatMesh(int nx, int ny) {
    vector<Vertex> vertices;
    vector<int> indices;

    for (int pz = 0; pz < ny; pz++) {
        for (int px = 0; px < nx; px++) {
            float x = float(px) / (float(ny) / 2.0f) - 1.0f;
            float y = 0.0;
            float z = float(pz) / (float(ny) / 2.0f) - 1.0f;

            vertices.emplace_back(x, y, z);
        }
    }

    for (int z = 0; z < ny - 1; z++) {
        for (int x = 0; x < nx - 1; x++) {
            int tl = z * nx + x;
            int tr = tl + 1;
            int bl = (z + 1) * nx + x;
            int br = bl + 1;
            indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
            indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
        }
    }

    return Mesh(vertices, indices);
}

DynamicMesh::DynamicMesh(vector<Vertex> v, vector<int> i) : Mesh() {
    vertices = v;
    indices = i;

    model = glm::mat4(1.0);

    setupDynamicMesh();
}

void DynamicMesh::setupDynamicMesh() {
    totalVertices += vertices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 nullptr, GL_STATIC_DRAW);

    calculateTangents();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    glBindVertexArray(0);
}

void DynamicMesh::updateSubData(vector<Vertex> v, int offset) {
    bind();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    for (int i = 0; i < v.size() && vertices.begin() + offset + i < vertices.end(); i++) {
        vertices[i + offset] = v[i];
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertices.size(), &vertices[0]);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DynamicMesh::updateSubData(vector<int> v, int offset) {
    bind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    for (int i = 0; i < v.size() && indices.begin() + offset + i < indices.end(); i++) {
        indices[i + offset] = v[i];
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Vertex) * indices.size(), &indices[0]);
}
