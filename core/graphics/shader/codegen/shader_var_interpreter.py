import os
import xml.etree.ElementTree as ET
import deque

from enum import Enum
class ShaderStageType(Enum):
    SHADER_STAGE_TYPE_VERTEX = 0
    SHADER_STAGE_TYPE_GEOMETRY = 1
    SHADER_STAGE_TYPE_FRAGMENT = 2
    SHADER_STAGE_TYPE_COMPUTE = 3

class GrammarOperatorType(Enum):
    CONCATENATION = 0
    ALTERNATION = 1
    OPTIONAL = 2
    REPETITION = 3
    GROUPING = 4

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
    def size(self) -> int:
        return None

    # Name of the data type
    def type_name(self) -> str:
        return None
    
    # List of tuples of (variable name, ShaderDataType) pairs
    def members(self) -> [(str, ShaderDataType)]:
        return None

    # Returns a non-None int if data type is an array.
    def array_length(self) -> int:
        return None
    
    def array_elements(self) -> [ShaderDataType]:
        return None

    def cpp_write(self) -> str:
        return None
        

class ShaderFundamentalDataType(ShaderDataType):
    __cpp_data_type = ""
    __size = 0
    
    def __init__(self, cpp_data_type : str, size : int):
        self.__cpp_data_type = cpp_data_type
        self.__size = size
        
    def size(self):
        return self.__size
    
    def type_name(self):
        return self.__cpp_data_type

class ShaderArrayDataType(ShaderDataType):
    __elements = []
    
    def __init__(self, elements : [ShaderDataType]):
        # TODO: constrain each element to be of same data type
        self.__elements = elements

    def type_name(self):
        return self.__elements[0].type_name()
        
    def size(self):
        return self.array_length() * self.__elements[0].size()

    def array_length(self):
        return len(self.__elements)

    def array_elements(self):
        return self.__elements

class ShaderStructDataType(ShaderDataType):
    __type_name = ""
    __members = []
    def __init__(self, type_name : str, members : [(str, ShaderDataType)]):
        self.__type_name = type_name
        self.__members = members
        
    def size(self):
        return None
    
    def type_name(self):
        return self.__type_name
    
    def members(self):
        return self.__members

    def array_length(self):
        return len(elements)

    def cpp_write(self):
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

def find_all_filepaths_in_directory_with_extension(root_path, extension, recursive=True):
    filepaths = []
    if(recursive):
        #TODO
    else:
        #TODO
    return None

def is_matching(nodes, grammar_rule : [str]):
    return None

def has_suffix_partial_grammar_match(stack : [SyntaxTreeNode], grammar):
    for i in range(1, len(stack)):
        for nonterminal_definition in grammar:
            for grammar_rule in nonterminal_definition[1]:
                if (is_matching(stack[-i:], grammar_rule[:i])):
                    return True
    return False

class ProductionRuleSymbolNode:
    # Either a single terminal character, or a number representing a nonterminal id
    __symbol = None
    __left_nodes = []
    __right_nodes = []
    
    def __init__(self, symbol : str, left_nodes : [ProductionRuleSymbolNode], right_nodes : [ProductionRuleSymbolNode]):
        self.__symbol = symbol
        self.__left_nodes = left_nodes
        self.__right_nodes = right_nodes
        
    def is_match(self, symbol):
        return self.__symbol == symbol

    def is_left_endpoint(self):
        return len(self.__left_nodes) == 0

    def is_right_endpoint(self):
        return len(self.__right_nodes) == 0

    def left_nodes(self):
        return self.__left_nodes

    def right_nodes(self):
        return self.__right_nodes

    def set_left_nodes(self, left_nodes):
        self.__left_nodes = left_nodes

    def set_right_nodes(self, right_nodes):
        self.__right_nodes = right_nodes

class Grammar:
    nonterminal_id_map = {}
    #reverse_grammar_fragment_lookup = []
    
    def __init__(self):
        return None

    def define_nonterminal(self, nonterminal_name : str, rule_start_node : LexicalSymbolNode, rule_end_node : ConnectorNode):
        
        return None

    def first_node_with_lexical_symbol(self, target_terminal_value : str):
        return None

    def find_descendant_nodes_with_nonterminal(self, root_node : SyntaxTreeNode, nonterminal : str) -> [SyntaxTreeNode]:
        bfs_queue = deque(root_node)
        matching_nonterminal_nodes = [root_node] if root_node.nonterminal_id == nonterminal_id else []
        
        while (len(bfs_queue) > 0)):
            node = bfs_queue.pop_front()
            if (node.nonterminal_id() == nonterminal_id):
                matching_nonterminal_nodes.append(node)
            for child_node in nodes.children():
                bfs_queue.push_back(child_node)
        return matching_nonterminal_nodes
        
    

# Using Extended Backus-Naur Form (EBNF)

def is_matching(nodes, grammar_rule : [str]):
    return None

def has_suffix_partial_grammar_match(stack : [SyntaxTreeNode], grammar):
    for i in range(1, len(stack)):
        for nonterminal_definition in grammar:
            for grammar_rule in nonterminal_definition[1]:
                if (is_matching(stack[-i:], grammar_rule[:i])):
                    return True
    return False

def tokenized_line(line : str):
    if (line == None):
        return None
    return line.split()

def is_grammar_nonterminal_definition_start(tokens : [str]):
    return len(tokens) >= 4 && tokens[1] == ':'

def is_grammar_nonterminal_definition_termination(tokens : [str]):
    return tokens[-1] == ';'

def is_terminal(token):
    return (token[0] == "'" and token[-1] == "'") or (token[0] == "\"" and token[-1] == "\"")

def parse_terminal(token) -> ([ProductionRuleSymbolNode], [ProductionRuleSymbolNode]):
    first_node = ProductionRuleSymbolNode(token[0], [], [])
    current_node = first_node
    for character in token[1:]:
        next_node = ProductionRuleSymbolNode(character, [current_node], [])
        current_node.set_right_nodes([next_node])
        current_node = next_node
    return ([first_node], [current_node])

def construct_concatenations(operands : [([ProductionRuleSymbolNode], [ProductionRuleSymbolNode])]) -> ([ProductionRuleSymbolNode], [ProductionRuleSymbolNode]):
    for i in range(len(operands)):
        for left_node in operands[i]:
            
    return None

def construct_alternations(operands: [([ProductionRuleSymbolNode], [ProductionRuleSymbolNode])]) -> ([ProductionRuleSymbolNode], [ProductionRuleSymbolNode]):
    return None

def construct_repetition(operand : ([ProductionRuleSymbolNode], [ProductionRuleSymbolNode])) -> ([ProductionRuleSymbolNode], [ProductionRuleSymbolNode]):
    return None

def construct_optional(operand : ([ProductionRuleSymbolNode], [ProductionRuleSymbolNode])) -> ([ProductionRuleSymbolNode], [ProductionRuleSymbolNode]):
    return ([None] + operand[0], [None] + operand[1])

def parse_alternations_and_concatenations(tokens):
    alternation_operands_tokens = []
    for token in tokens:
        if (token == "|"):
            alternated_operands_tokens.append([])
        else:
            if (len(alternated_operands_tokens[-1]) % 2 == 1):
                assert token == ",", "Concatenated tokens must be separated by a ','"
            else:
                alternated_operands_tokens[-1].append(token)
            
    alternation_operands = []
    for alternation_operand_tokens in alternation_operands_tokens:
        alternation_operands.append(construct_concatenations(alternation_operand_tokens))
    return construct_alternations(alternation_operands)

def parse_production_rule(rule_tokens):
    grouping_stack = [(GrammarOperatorType.GROUPING, [])]
    for token in rule_tokens:
        if (is_terminal(token)):
            grouping_stack[-1][1].append(parse_terminal(token))
        elif (token == "("):
            grouping_stack.append((GrammarOperatorType.GROUPING, []))
        elif (token == "{"):
            grouping_stack.append((GrammarOperatorType.REPETITION, []))
        elif (token == "["):
            grouping_stack.append((GrammarOperatorType.OPTIONAL, []))
        elif(token == ")"):
            operator_type, group_tokens = grouping_stack.pop()
            assert operator_type == GrammarOperatorType.GROUPING
            grouping_stack[-1][1].append(parse_alternations_and_concatenations(group_tokens))
        elif (token == "}"):
            operator_type, repetition_tokens = grouping_stack.pop()
            assert operator_type == GrammarOperatorType.REPETITION
            repetition_left_right_nodes = parse_alternations_and_concatenations(repetition_tokens)
            grouping_stack[-1][1].append(construct_repetition(repetition_left_right_nodes))
        elif (token == "]"):
            operator_type, optional_tokens = grouping_stack.pop()
            assert operator_type == GrammarOperatorType.OPTIONAL
            optional_left_right_nodes = parse_alternations_and_concatenations(optional_tokens)
            grouping_stack[-1][1].append(construct_optional(optional_left_right_nodes))
        else:
            grouping_stack[-1][1].append(token)
    
    assert(len(grouping_stack) == 1)
    return parse_alternations_and_concatenations(grouping_stack[0][1])

def load_shader_language_ebnf_grammar(filepath : str):
    grammar = Grammar()

    grammar_file = open(filepath, 'r')
    grammar_file_line = grammar_file.readline()
    while(grammar_file_line):
        line_tokens = tokenized_line(grammar_file_line)
        if (is_grammar_nonterminal_definition_start(line_tokens)):
            # Start of a new grammar nonterminal definition
            grammar.append([])

            # Find semicolon terminating the nonterminal definition.
            nonterminal_definition_tokens = line_tokens
            while (not is_grammar_nonterminal_definition_termination(nonterminal_definition_tokens)):
                nonterminal_definition_tokens.extend(tokenized_line(grammar_file.readline()))

            nonterminal_name = nonterminal_definition_tokens[0]
            rule_start_node, rule_end_node = parse_production_rule(nonterminal_definition_tokens[2:-1])
        else:
            grammar_file_line = grammar_file.readline()
    
    grammar_file.close()

    # Iterate through grammar fragments and replace nonterminal names with ids and special token names with corresponding characters.
    for grammar_nonterminal_rules in grammar:
        for rule in grammar_nonterminal_rules:
            for token_index, token in enumerate(rule):
                if (token.isupper()):
                    # Terminal token
                    if (token is in special_tokens_map):
                        rule[token_index] = special_tokens_map[token]
                    else:
                        rule[token_index] = token.lower()
                else:
                    # Nonterminal token
                    rule[token_index] = grammar_fragment_id_map[token]             
    return grammar

def reduce_suffix(stack : [SyntaxTreeNode], grammar):
    partial_matching_grammar = [(nonterminal_id, grammar_rules) for nonterminal_id, grammar_rules in enumerate(grammar)]
    for i in range(1, len(stack)):
        suffix = stack[-i:]
        updated_partial_matching_grammar = []
        for nonterminal_definition in partial_matching_grammar:
            partial_matching_grammar_rules = []
            for grammar_rule in nonterminal_definition[1]:
                if (len(suffix) == len(grammar_rule)
                    && is_matching(suffix, grammar_rule)):
                    # We have a match. Reduce
                    stack[-i] = SyntaxTreeNonTerminalNode()
                    return True    
                elif (len(suffix) < len(grammar_rule)
                      && len(stack) >= len(grammar_rule)
                      && is_matching(suffix, grammar_rule[-len(suffix):])):
                    # Potential candidate for matching later one
                    partial_matching_grammar_rules.append(grammar_rule)
            if (len(candidate_grammar_rules) > 0):
                updated_partial_matching_grammar.append((nonterminal_definition[0], partial_matching_grammar_rules))
        partial_matching_grammar = updated_partial_matching_grammar
    return False

def reduce(stack : [SyntaxTreeNode], grammar):
    while (True):
        if (not reduce_suffix(stack, grammar)):
            return;

def shift_reduce_parse_shader_code_syntax(shader_code : str, grammar):
    tokenized_shader_code = shader_code.split()
    stack = [tokenized_shader_code[0]]
    shift_offset = 1
    while(shift_offset < len(tokenized_shader_code))
        lookahead = tokenized_shader_code[shift_offset]
        if (has_suffix_partial_grammar_match(stack + [lookahead], grammar)):
            # shift
            stack.append(lookahead)
            shift_offset += 1
        else:
            # reduce
            reduce(stack, grammar)
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

        syntax_tree = shift_reduce_parse_shader_code_syntax(shader_stage_code, grammar)
                    
                
        
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
            
        
