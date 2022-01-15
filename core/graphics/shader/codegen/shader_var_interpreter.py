import os
import xml.etree.ElementTree as ET

from enum import Enum
class ShaderStageType(Enum):
    SHADER_STAGE_TYPE_VERTEX = 0
    SHADER_STAGE_TYPE_GEOMETRY = 1
    SHADER_STAGE_TYPE_FRAGMENT = 2
    SHADER_STAGE_TYPE_COMPUTE = 3

class Uniform:
    name = ""
    data_type = None
    location = -1
    array_length = 0

class VertexAttribute:
    name = ""
    data_type = None
    location = -1
    dimension = 0
    format = 0
    usage_category = 0

class ShaderDataType:
    # Size of data type, in bytes
    def size() -> int:
        return None

    # Name of the data type
    def type_name() -> str:
        return None
    
    # List of tuples of (variable name, ShaderDataType) pairs
    def members() -> [(str, ShaderDataType)]:
        return None

    # Returns a non-None int if data type is an array.
    def array_length() -> int:
        return None
    
    def array_elements() -> [ShaderDataType]:
        return None

    def cpp_write() -> str:
        return None
        

class ShaderFundamentalDataType(ShaderDataType):
    __cpp_data_type = ""
    __size = 0
    
    def __init__(self, cpp_data_type : str, size : int):
        self.__cpp_data_type = cpp_data_type
        self.__size = size
        
    def size():
        return self.__size
    
    def type_name():
        return self.__cpp_data_type

class ShaderArrayDataType(ShaderDataType):
    __elements = []
    
    def __init__(self, elements : [ShaderDataType]):
        # TODO: constrain each element to be of same data type
        self.__elements = elements

    def type_name():
        return self.__elements[0].type_name()
        
    def size():
        return self.array_length() * self.__elements[0].size()

    def array_length():
        return len(self.__elements)

    def array_elements():
        return self.__elements

class ShaderStructDataType(ShaderDataType):
    __type_name = ""
    __members = []
    def __init__(self, type_name : str, members : [(str, ShaderDataType)]):
        self.__type_name = type_name
        self.__members = members
        
    def size():
        return None
    
    def type_name():
        return self.__type_name
    
    def members():
        return self.__members

    def array_length():
        return len(elements)

    def cpp_write():
        output = "struct {0}\n{\n".format(self.type_name())
        for member in self.members():
            member_name = member[0]
            member_data_type = member[1]
            if(member_data_type.array_length() != None):
                # Member is an array
                output += "{0} {1}".format(member_name, member_data_type.type_name())
                # This while loop handles multidimensional arrays
                element_data_type = member_data_type
                while(element_data_type.array_length() != None):
                    assert(element_data_type == member_data_type.type_name())
                    assert(element_data_type.array_length() > 0)
                    output += "[{0}]".format(element_data_type.array_length())
                    element_data_type = element_data_type.array_elements()[0]
                output += ";\n"
            else:
                output += "{0} {1};\n".format(member_name, member_data_type.type_name())
        output += "};\n"
        return output
            

fundamental_glsl_data_types_map = \
{
    "bool" :  ShaderFundamentalDataType("bool", 1),
    "bvec2" : ShaderFundamentalDataType("glm::bvec2", 8),
    "bvec3" : ShaderFundamentalDataType("glm::bvec3", 12),
    "bvec4" : ShaderFundamentalDataType("glm::bvec4", 16),
    
    "float" : ShaderFundamentalDataType("float", 4),
    "vec2" : ShaderFundamentalDataType("glm::vec2", 8),
    "vec3" : ShaderFundamentalDataType("glm::vec3", 12),
    "vec4" : ShaderFundamentalDataType("glm::vec4", 16),
    
    "int" : ShaderFundamentalDataType("int", 4),
    "ivec2" : ShaderFundamentalDataType("glm::ivec2", 8),
    "ivec3" : ShaderFundamentalDataType("glm::ivec3", 12),
    "ivec4" : ShaderFundamentalDataType("glm::ivec4", 16),
    
    "uint" : ShaderFundamentalDataType("uint", 4),
    "uvec2" : ShaderFundamentalDataType("glm::uvec2", 8),
    "uvec3" : ShaderFundamentalDataType("glm::uvec3", 12),
    "uvec4" : ShaderFundamentalDataType("glm::uvec4", 16),
}

storage_qualifiers = {"uniform", "in", "out", "buffer"}

special_tokens_map = \
{
    "COMMA" : ",",
    "SEMICOLON" : ";",
    "LEFT_PAREN" : "(",
    "RIGHT_PAREN" : ")",
    "LEFT_BRACKET" : "[",
    "RIGHT_BACKET" : "]",
    "LEFT_BRACE" : "{",
    "RIGHT_BRACE" : "}"
}

def tokenized_line(line : str):
    if (line == None):
        return None
    return line.split()

def is_grammar_fragment_declaration(tokens : [str]):
    return len(tokens) == 2 && tokens[-1] == ':'

def load_shader_language_grammar(filepath : str):
    grammar = []
    grammar_fragment_id_map = {}
    #reverse_grammar_fragment_lookup = []

    grammar_file = open(filepath, 'r')
    grammar_file_line = grammar_file.readline()
    while(grammar_file_line):
        line_tokens = tokenized_line(grammar_file_line)
        if (is_grammar_fragment_declaration(line_tokens)):
            # Start of a new grammar fragment
            fragment_id = len(grammar_fragment_id_map)
            grammar_fragment_id_map[line_tokens[0]] = fragment_id
            assert len(grammar) == fragment_id , "Fragment id must match index in grammar."
            grammar.append([])

            grammar_fragment_line_tokens = tokenized_line(grammar_file.readline())
            while (not is_grammar_fragment_declaration(grammar_fragment_line_tokens) and len(grammar_fragment_line_tokens) > 0):
                grammar[-1].append(grammar_fragment_line_tokens)
                grammar_fragment_line_tokens = tokenized_line(grammar_file.readline())
        else:
            grammar_file_line = grammar_file.readline()
    
    grammar_file.close()

    # Iterate through grammar fragments and replace fragment names with ids and special token names with corresponding characters.
    for i in range(len(grammar)):
        grammar_fragment = grammer[i]
        for j in range(len(grammar_fragment)):
            token = grammar_fragment[j]
            if (token.isupper()):
                if (token is in special_tokens_map):
                    grammar[i][j] = special_tokens_map[token]
                else:
                    grammar[i][j] = token.lower()
            else:
                grammar[i][j] = grammar_fragment_id_map[token]             
    return grammar

def find_all_filepaths_in_directory_with_extension(root_path, extension, recursive=True):
    filepaths = []
    if(recursive):
        #TODO
    else:
        #TODO
    return None

def parse_shader_code_syntax(shader_code : str, grammar):
    # TODO
    return None

def parse_rendering_pipeline_uniforms_and_vertex_attributes(rendering_pipeline_path : str, grammar):
    """
    Uniforms may be declared in any of the shaders stages.

    Vertex attributes are the inputs of the vertex shader, and are the only inputs that can
    be modified by the game engine renderer.
    """
    
    rendering_pipeline_xml_tree = ET.parse(rendering_pipeline_path)
    root = rendering_pipeline_xml_tree.getroot()

    for shader_stage_node in shader_stages:
        shader_stage_type = shader_stage_node["stage_type"]
        shader_stage_code = shader_stage_node["code"]

        syntax_tree = parse_shader_code_syntax(shader_stage_code, grammar)
                    
                
        
def generate_cpp_structs_from_rendering_pipelines(engine_resources_path, game_resources_path):
    grammar = load_shader_language_grammer("./shader_language_grammar.txt")
    rendering_pipelines_filepaths = [find_all_file_paths_in_directory_with_extension(engine_resources_path, "rp")]
    rendering_pipelines_filepaths.extend(find_all_file_paths_in_directory_with_extension(game_resources_path, "rp"))

    for rendering_pipeline_path in rendering_pipelines_filepaths:
        uniforms, vertex_attributes = parse_rendering_pipeline_uniforms_and_vertex_attributes(rendering_pipeline_path, grammar)

"""
def build_component_from_spec(component_spec_filepath, component_type_id):
    component_spec_file = open(component_spec_filepath, 'r')
    component_file_contents = []
    #parse imports
    import_line = component_spec_file.readline()
    import_path_pair = import_line.split(' ')
    while(import_path_pair[0] == '#import'):
        if (len(import_path_pair) != 2):
            raise RuntimeError('Please denote imported files with the format: `#import "FILE_PATH"`') 
        component_file_contents.append("#include " + import_path_pair[1])
        import_line = component_spec_file.readline()
        import_path_pair = import_line.split(" ")
    
    #parse component declaration
    component_declaration = import_line
    if (not component_declaration.endswith('Spec')):
        raise RuntimeError('Component spec name declaration must end in "Spec"')
    component_name = component_declaration[:-4]
    component_file_contents.append("struct " + component_name)
    component_file_contents.append("{")
    
    #parse member vars
    # TODO: Parse public and then private members
    component_file_contents.append("static const std::uint32_t type_id;")
    
    member_var_line = component_spec_file.readline()
    while(member_var_line):
        type_var_pair = member_var_line.split(' ')
        if (len(type_var_pair) != 2):
            raise RuntimeError('Please denote component member variables with the format: "TYPE VAR_NAME"')
        component_file_contents.append("\t" + import_path_pair[0] + " " + import_path_pair[1] + ";")
        member_var_line = component_spec_file.readline()
        
    component_spec_file.close()
    component_file_contents.append("};")
    component_file_contents.append("")
    component_file_contents.append("std::uint32_t " + component_name + "::type_id = " + component_type_id)

    component_file = open(os.path.join(, , ".h"), 'w')
    for component_file_line in component_file_contents:
        component_file.write(component_file_line + "\n")
    component_file.close()
    return None
"""

if __name__ == '__main__':
    engine_core_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '../core')
    component_type_id = 1
    for subdir, dirs, filenames in os.walk(engine_core_path):
        for filename in filenames:
            filepath = os.path.join(subdir, filename)
            print(filepath)
            filename, extension = os.path.splitext(filename)
            print(extension)
            if (extension == 'cspec'):
                build_component_from_spec(filepath, component_type_id)
                component_type_id += 1
            
        
