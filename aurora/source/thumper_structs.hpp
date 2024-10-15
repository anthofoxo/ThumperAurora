#pragma once

#include "vector_stream.hpp"

#include <glm/glm.hpp>

#include <cstdint>
#include <optional>
#include <span>
#include <filesystem>

namespace thumper {
    struct Vertex final {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        glm::u8vec4 color;
    };

    struct Triangle {
        uint16_t elements[3];
    };

    struct Mesh final {
        std::vector<Vertex> vertices;
        std::vector<Triangle> triangles;
        uint16_t _unknownField4;
    };

    struct MeshFile final {
        std::vector<Mesh> meshes;

        static std::optional<MeshFile> from_file(std::filesystem::path const& path) {
            auto stream = aurora::VectorStream::from_file(path);
            if (!stream) return std::nullopt;
            return deserialize(*stream);
        }

        void to_file(std::filesystem::path const& path) const {
            serialize().to_file(path);
        }

        static std::optional<MeshFile> deserialize(aurora::VectorStream& stream) {
            if (stream.read_u32() != 6) return std::nullopt; // Header check
            uint32_t meshCount = stream.read_u32();

            if (meshCount == 0) return std::nullopt; // No meshes to read
            if (meshCount == 167 || meshCount == 174) return std::nullopt; // Special values as part of other headers, the game never uses this many LODs

            MeshFile file;
            file.meshes.resize(meshCount);

            for (int i = 0; i < meshCount; ++i) {
                uint32_t vertexCount = stream.read_u32();
                auto verticesBytes = stream.read_bytes(vertexCount * sizeof(Vertex));
                file.meshes[i].vertices.resize(vertexCount);
                memcpy(file.meshes[i].vertices.data(), verticesBytes.data(), verticesBytes.size_bytes());

                uint32_t triangleCount = stream.read_u32();
                auto trianglesBytes = stream.read_bytes(triangleCount * sizeof(Triangle));
                file.meshes[i].triangles.resize(triangleCount);
                memcpy(file.meshes[i].triangles.data(), trianglesBytes.data(), trianglesBytes.size_bytes());

                file.meshes[i]._unknownField4 = stream.read_u16();
            }

            return file;
        }

        aurora::VectorStream serialize() const {
            aurora::VectorStream stream;
            stream.write_u32(6); // Header
            stream.write_u32(meshes.size());

            for (auto const& mesh : meshes) {
                stream.write_u32(mesh.vertices.size());
                stream.write_bytes(std::as_bytes(std::span(mesh.vertices)));
                stream.write_u32(mesh.triangles.size());
                stream.write_bytes(std::as_bytes(std::span(mesh.triangles)));
                stream.write_u16(mesh._unknownField4);
            }

            return stream;
        }
    };
}