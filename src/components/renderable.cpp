#include "components/renderable.hpp"
#include "graphics/shader.hpp"
#include "resources/mesh.hpp"
#include "graphics/texture.hpp"
#include "systems/resource-system.hpp"

namespace trillek {
namespace graphics {

Renderable::Renderable() { }
Renderable::~Renderable() { }

void Renderable::UpdateBufferGroups() {
    // Check if the mesh is valid and assign an empty one if it isn't.
    if (!this->mesh) {
        return;
    }

    for (size_t i = 0; i < this->mesh->GetMeshGroupCount(); ++i) {
        std::shared_ptr<BufferGroup> buffer_group;
        if (i < this->buffer_groups.size()) {
            buffer_group = this->buffer_groups[i];
        }
        else {
            buffer_group = std::make_shared<BufferGroup>();
            glGenVertexArrays(1, &buffer_group->vao); // Generate the VAO
            glGenBuffers(1, &buffer_group->vbo); 	// Generate the vertex buffer.
            glGenBuffers(1, &buffer_group->ibo); // Generate the element buffer.
            this->buffer_groups.push_back(buffer_group);
        }

        std::weak_ptr<resource::MeshGroup> mesh_group = this->mesh->GetMeshGroup(i);
        glBindVertexArray(buffer_group->vao); // Bind the VAO

        auto temp_meshgroup = mesh_group.lock();

        // TODO: Loop through all the texture names in the mesh group and add the textures to the material.
        for (std::string texture_name : temp_meshgroup->textures) {
            std::shared_ptr<Texture> texture = resource::ResourceMap::Get<Texture>(texture_name);
            if (!texture) {
                std::vector<Property> props;
                props.push_back(Property("filename", texture_name));
                auto pixel_data = resource::ResourceMap::Create<resource::PixelBuffer>(texture_name, props);
                if (pixel_data) {
                    texture = std::make_shared<Texture>(*pixel_data.get());
                    resource::ResourceMap::Add<Texture>(texture_name, texture);
                }
            }

            if (texture) {
                buffer_group->textures.push_back(texture);
            }
        }

        if (temp_meshgroup) {
            if (temp_meshgroup->verts.size() > 0) {

                glBindBuffer(GL_ARRAY_BUFFER, buffer_group->vbo); // Bind the vertex buffer.
                glBufferData(GL_ARRAY_BUFFER, sizeof(resource::VertexData) * temp_meshgroup->verts.size(),
                    &temp_meshgroup->verts[0], GL_STATIC_DRAW); // Stores the verts in the vertex buffer.

                GLuint shader_program = 0;
                if (this->shader) {
                    shader_program = this->shader->GetProgram();
                }

                GLuint posLocation = glGetAttribLocation(shader_program, "pos");
                glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, position)); // Tell the VAO the vertex data will be stored at the location we just found.
                glEnableVertexAttribArray(posLocation); // Enable the VAO line for vertex data.

                GLuint normalLocation = glGetAttribLocation(shader_program, "norm");
                glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, normal));
                glEnableVertexAttribArray(normalLocation);

                GLuint colLocation = glGetAttribLocation(shader_program, "color");
                glVertexAttribPointer(colLocation, 4, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, color));
                glEnableVertexAttribArray(colLocation);

                GLuint uvLocation = glGetAttribLocation(shader_program, "tex1");
                glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, uv)); // Tell the VAO the vertex data will be stored at the location we just found.
                glEnableVertexAttribArray(uvLocation); // Enable the VAO line for vertex data.
            }

            if (temp_meshgroup->indicies.size() > 0) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_group->ibo); // Bind the element buffer.
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* temp_meshgroup->indicies.size(),
                    &temp_meshgroup->indicies[0], GL_STATIC_DRAW); // Store the faces in the element buffer.
                buffer_group->ibo_count = temp_meshgroup->indicies.size();
            }
        }

        glBindVertexArray(0); // Reset the buffer binding because we are good programmers.

    }
}

void Renderable::SetMesh(std::shared_ptr<resource::Mesh> m) {
    this->mesh = m;
}

std::shared_ptr<resource::Mesh> Renderable::GetMesh() const {
    return this->mesh;
}

void Renderable::SetShader(std::shared_ptr<Shader> m) {
    this->shader = m;
}

std::shared_ptr<Shader> Renderable::GetShader() const {
    return this->shader;
}

bool Renderable::Initialize(const std::vector<Property> &properties) {
    std::string mesh_name;
    std::string shader_name;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "mesh") {
            mesh_name = p.Get<std::string>();
        }
        else if (name == "shader") {
            shader_name = p.Get<std::string>();
        }
    }

    this->mesh = resource::ResourceMap::Get<resource::Mesh>(mesh_name);
    if (!mesh) {
        return false;
    }

    this->shader = resource::ResourceMap::Get<graphics::Shader>(shader_name);
    if (!shader) {
        return false;
    }

    UpdateBufferGroups();

    return true;
}

} // End of graphics
} // End of trillek
