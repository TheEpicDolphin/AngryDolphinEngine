#pragma once

#include "archive.h"

namespace serializable {
    template <typename... Args>
    std::vector<ArchiveSerNodeBase*> RegisterSerializableMembers(Archive& archive, rapidxml::xml_document<>& xml_doc, const char* comma_separated_names, Args&... args) {
        const std::size_t member_count = sizeof...(Args);
        std::string names[member_count];
        const char* arg_name_location = comma_separated_names;
        for (int i = 0; i < member_count; ++i) {
            const char* comma_location = strchr(arg_name_location, ',');
            if (comma_location == nullptr) {
                names[i] = std::string(arg_name_location);
            } else {
                names[i] = std::string(arg_name_location, comma_location - arg_name_location);
                arg_name_location = comma_location + 1;
                // Skip whitespace
                while ((*arg_name_location != '\0') && std::isspace(*arg_name_location) > 0) {
                    arg_name_location++;
                }
            }
        }

        int child_index = 0;
        return { (archive.RegisterObjectForSerialization(xml_doc, names[child_index++], args))... };
    }

    template <typename... Args>
    std::vector<ArchiveDesNodeBase*> RegisterDeserializableMembers(Archive& archive, rapidxml::xml_node<>& parent_xml_node, Args&... args) {
        std::vector<rapidxml::xml_node<>*> children_xml_nodes;
        rapidxml::xml_node<>* child_node = parent_xml_node.first_node();
        while (child_node) {
            children_xml_nodes.push_back(child_node);
            child_node = child_node->next_sibling();
        }

        int child_index = 0;
        return { (archive.RegisterObjectForDeserialization(*children_xml_nodes[child_index++], args))... };
    }
}

#define SERIALIZE_MEMBERS(...) std::vector<ArchiveSerNodeBase*> RegisterSerializableMembers(Archive& archive, rapidxml::xml_document<>& xml_doc){ \
    return serializable::RegisterSerializableMembers(archive, xml_doc, #__VA_ARGS__, __VA_ARGS__); \
} \
\
std::vector<ArchiveDesNodeBase*> RegisterDeserializableMembers(Archive& archive, rapidxml::xml_node<>& parent_xml_node){ \
    return serializable::RegisterDeserializableMembers(archive, parent_xml_node, __VA_ARGS__); \
} \
