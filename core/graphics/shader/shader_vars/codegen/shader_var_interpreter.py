import os

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
        

class ShaderScalarDataType(ShaderDataType):
    __glm_data_type = ""
    __size = 0
    
    def __init__(self, __glm_data_type : str, size : int):
        self.__glm_data_type = __glm_data_type
        self.__size = size
        
    def size():
        return self.__size
    
    def type_name():
        return self.__glm_data_type

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
    def __init__(self, type_name : str, members : [ShaderDataType]):
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
            

built_in_glsl_data_types_map =
{
    "bool" :  ShaderScalarDataType("bool", 1),
    "bvec2" : ShaderScalarDataType("glm::bvec2", 8),
    "bvec3" : ShaderScalarDataType("glm::bvec3", 12),
    "bvec4" : ShaderScalarDataType("glm::bvec4", 16),
    
    "float" : ShaderScalarDataType("float", 4),
    "vec2" : ShaderScalarDataType("glm::vec2", 8),
    "vec3" : ShaderScalarDataType("glm::vec3", 12),
    "vec4" : ShaderScalarDataType("glm::vec4", 16),
    
    "int" : ShaderScalarDataType("int", 4),
    "ivec2" : ShaderScalarDataType("glm::ivec2", 8),
    "ivec3" : ShaderScalarDataType("glm::ivec3", 12),
    "ivec4" : ShaderScalarDataType("glm::ivec4", 16),
    
    "uint" : ShaderScalarDataType("uint", 4),
    "uvec2" : ShaderScalarDataType("glm::uvec2", 8),
    "uvec3" : ShaderScalarDataType("glm::uvec3", 12),
    "uvec4" : ShaderScalarDataType("glm::uvec4", 16),
}
        

class ShaderVarNode:
    type_name = ""
    var_name = ""
    children = []

def find_all_filepaths_in_directory_with_extension(root_path, extension, recursive=True):
    filepaths = []
    if(recursive):

    else:
        
    return None

def parse_rendering_pipeline_uniforms_and_vertex_attributes(rendering_pipeline_path):
    """
    Uniforms may be declared in any of the shaders stages.

    Vertex attributes are the inputs of the vertex shader, and are the only inputs that can
    be modified by the game engine renderer.
    """
    
    ast = parse_shader_ast(rendering_pipeline_path)
    rendering_pipeline_file = open(rendering_pipeline_path, 'r')
    while():
        shader_line = component_spec_file.readline()

    rendering_pipeline_file.close()
        
def generate_cpp_structs_from_rendering_pipelines(engine_resources_path, game_resources_path):
    rendering_pipelines_filepaths = [find_all_file_paths_in_directory_with_extension(engine_resources_path, "rp")]
    rendering_pipelines_filepaths.extend(find_all_file_paths_in_directory_with_extension(game_resources_path, "rp"))

    for rendering_pipeline_path in rendering_pipelines_filepaths:
        uniforms, vertex_attributes = parse_rendering_pipeline_uniforms_and_vertex_attributes(rendering_pipeline_path)

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
            
        
