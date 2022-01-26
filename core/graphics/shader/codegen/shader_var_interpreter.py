import os
import xml.etree.ElementTree as ET
import deque
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

    def cpp_write(self) -> str:
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

class NonTerminalSymbolNode:
    __nonterminal_name = None
    __nonterminal_id = None
    __children = []

    def __init__(self, nonterminal_name : str, nonterminal_id : int, children):
        self.__nonterminal_name = nonterminal_name
        self.__nonterminal_id = nonterminal_id
        self.__children = children

    def first_node_with_terminal_symbol(self, terminal_symbol : str):
        return None

    def find_descendant_nodes_with_nonterminal_symbol(self, root_node : SyntaxTreeNode, nonterminal : str) -> [SyntaxTreeNode]:
        bfs_queue = deque(root_node)
        matching_nonterminal_nodes = [root_node] if root_node.nonterminal_id == nonterminal_id else []
        
        while (len(bfs_queue) > 0)):
            node = bfs_queue.pop_front()
            if (node.nonterminal_id() == nonterminal_id):
                matching_nonterminal_nodes.append(node)
            for child_node in nodes.children():
                bfs_queue.push_back(child_node)
        return matching_nonterminal_nodes

    def nonterminal_name(self):
        return self.__nonterminal_name
    
    def nonterminal_id(self):
        return self.__nonterminal_id
    
    def children(self):
        return self.__children

# Extended Backus-Naur Form (EBNF) Grammar
class EBNFGrammar:
    __nonterminal_id_map = {}
    __nonterminal_name_lookup = []
    __nonterminal_production_rules = []

    class _GrammarOperatorType(Enum):
        CONCATENATION = 0
        ALTERNATION = 1
        OPTIONAL = 2
        REPETITION = 3
        GROUPING = 4

    class _ProductionRuleSymbolNode:
        # Either a single terminal character, or a number representing a nonterminal id
        __symbol = None
        left_nodes = []
        right_nodes = []
        
        def __init__(self, symbol : str, left_nodes : [ProductionRuleSymbolNode], right_nodes : [ProductionRuleSymbolNode]):
            self.__symbol = symbol
            self.left_nodes = left_nodes
            self.right_nodes = right_nodes
            
        def is_match(self, symbol):
            return self.__symbol == symbol

        def is_left_endpoint(self):
            return len(self.left_nodes) == 0

        def is_right_endpoint(self):
            return len(self.right_nodes) == 0
    
    def __init__(self, grammar_filepath : str):
        self.load_grammar_file(grammar_filepath)

    def __tokenized_line(line : str):
    if (line == None):
        return None
    return line.split()

    def __is_grammar_nonterminal_definition_start(tokens : [str]):
        return len(tokens) >= 4 && tokens[1] == ':'

    def __is_grammar_nonterminal_definition_termination(tokens : [str]):
        return tokens[-1] == ';'

    def __is_terminal(token):
        return len(token) == 3 and ((token[0] == "'" and token[-1] == "'") or (token[0] == "\"" and token[-1] == "\""))

    def __is_terminal_sequence(token):
        return len(token) > 3 and ((token[0] == "'" and token[-1] == "'") or (token[0] == "\"" and token[-1] == "\""))

    def __is_nonterminal(self, token):
        return token is in self.__nonterminal_id_map

    def __parse_terminal(token : str) -> ([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode]):
        assert len(token) == 1
        return ProductionRuleSymbolNode(token[0], [], [])

    def __parse_terminal_sequence(token : str) -> ([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode]):
        assert len(token) > 1
        first_node = ProductionRuleSymbolNode(token[0], [], [])
        current_node = first_node
        for character in token[1:]:
            next_node = _ProductionRuleSymbolNode(character, [current_node], [])
            current_node.right_nodes = [next_node]
            current_node = next_node
        return ([first_node], [current_node])


    def __parse_nonterminal(self, token : str) -> ([ProductionRuleSymbolNode], [ProductionRuleSymbolNode]):
        node = _ProductionRuleSymbolNode(self.__nonterminal_id_map[token], [], [])
        return ([node], [node])
        
    def __construct_concatenations(operands : [([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode])]) -> ([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode]):
        concatenation = ([], operands[-1][1])

        # Handle the case of an optional operand on the far left
        for operand in operands:
            operand_is_optional = False
            for left_node in operand[0]:
                if (left_node):
                    concatenation[0].append(left_node)
                else:
                    # This operand is optional
                    operand_is_optional = True
            if (not operand_is_optional):
                break;

        # Connect adjacent operand nodes
        for i in range(0, len(operands) - 1):
            left_operand = operands[i]
            for right_end_node in left_operand[1]:
                for right_operand in operands[i + 1:]:
                    right_operand_is_optional = False
                    for left_end_node in right_operand[0]:
                        if (left_end_node):
                            left_end_node.left_nodes.extend(left_operand[1])
                            right_end_node.right_nodes.append(left_end_node)
                        else:
                            # This right_operand is optional
                            right_operand_is_optional = True
                    if (not right_operand_is_optional):
                        break;
                        
        return concatenation

    def __construct_alternations(operands: [([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode])]) -> ([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode]):
        left_end_nodes = []
        right_end_nodes = []
        for operand in operands:
            left_end_nodes.extend(operand[0])
            right_end_nodes.extend(operand[1])
        return (left_nodes, right_nodes)

    def __construct_repetition(operand : ([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode])) -> ([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode]):
        for right_end_node in operand[1]:
            right_end_node.extend(operand[0])
            
        for left_end_node in operand[0]:
            # Note that 'None' can only appear as a left end node
            if (left_end_node):
                left_end_node.left_nodes.extend(operand[1])

        return ([None] + operand[0], operand[1])

    def __construct_optional(operand : ([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode])) -> ([_ProductionRuleSymbolNode], [_ProductionRuleSymbolNode]):
        # If there are any 'None's left after the parsing, then the production rule incorrectly allows an empty string match.
        return ([None] + operand[0], operand[1])

    def __parse_any_alternations_and_concatenations(tokens):
        alternation_operands_tokens = []
        for token in tokens:
            if (token == "|"):
                alternated_operands_tokens.append([])
            else:
                if (len(alternated_operands_tokens[-1]) % 2 == 1):
                    assert token == ",", "Concatenated tokens must be separated by a \',\'"
                else:
                    alternated_operands_tokens[-1].append(token)
                
        alternation_operands = []
        for alternation_operand_tokens in alternation_operands_tokens:
            alternation_operands.append(__construct_concatenations(alternation_operand_tokens))
        return __construct_alternations(alternation_operands)

    def __parse_production_rule(self, rule_tokens):
        grouping_stack = [(_GrammarOperatorType.GROUPING, [])]
        for token in rule_tokens:
            if (__is_terminal(token)):
                grouping_stack[-1][1].append(__parse_terminal(token[1:-1]))
            elif(__is_terminal_sequence(token)):
                grouping_stack[-1][1].append(__parse_terminal_sequence(token[1:-1]))
            elif(self.__is_nonterminal(token)):
                grouping_stack[-1][1].append(__parse_nonterminal(token))
            elif (token == "("):
                grouping_stack.append((_GrammarOperatorType.GROUPING, []))
            elif (token == "{"):
                grouping_stack.append((_GrammarOperatorType.REPETITION, []))
            elif (token == "["):
                grouping_stack.append((_GrammarOperatorType.OPTIONAL, []))
            elif(token == ")"):
                operator_type, group_tokens = grouping_stack.pop()
                assert operator_type == _GrammarOperatorType.GROUPING
                grouping_stack[-1][1].append(__parse_any_alternations_and_concatenations(group_tokens))
            elif (token == "}"):
                operator_type, repetition_tokens = grouping_stack.pop()
                assert operator_type == _GrammarOperatorType.REPETITION
                repetition_left_right_nodes = __parse_any_alternations_and_concatenations(repetition_tokens)
                grouping_stack[-1][1].append(__construct_repetition(repetition_left_right_nodes))
            elif (token == "]"):
                operator_type, optional_tokens = grouping_stack.pop()
                assert operator_type == _GrammarOperatorType.OPTIONAL
                optional_left_right_nodes = self.__parse_any_alternations_and_concatenations(optional_tokens)
                grouping_stack[-1][1].append(__construct_optional(optional_left_right_nodes))
            else:
                grouping_stack[-1][1].append(token)
        
        assert(len(grouping_stack) == 1)
        return __parse_any_alternations_and_concatenations(grouping_stack[0][1])

    def load_grammar_file(self, filepath : str):
        nonterminal_definitions = []
        grammar_file = open(filepath, 'r')
        grammar_file_line = grammar_file.readline()
        while(grammar_file_line):
            line_tokens = tokenized_line(grammar_file_line)
            if (__is_grammar_nonterminal_definition_start(line_tokens)):
                # Start of a new grammar nonterminal definition
                # Find semicolon terminating the nonterminal definition.
                nonterminal_definition_tokens = line_tokens
                while (not __is_grammar_nonterminal_definition_termination(nonterminal_definition_tokens)):
                    nonterminal_definition_tokens.extend(tokenized_line(grammar_file.readline()))

                nonterminal_name = nonterminal_definition_tokens[0]
                nonterminal_id = len(self.__nonterminal_id_map)
                self.__nonterminal_id_map[nonterminal_name] = nonterminal_id
                assert len(self.__nonterminal_name_lookup) == nonterminal_id
                self.__nonterminal_name_lookup.append(nonterminal_name)
                nonterminal_definitions.append((nonterminal_name, nonterminal_definition_tokens[2:-1]))
            else:
                grammar_file_line = grammar_file.readline()
        grammar_file.close()

        for (nonterminal_name, nonterminal_production_rule_tokens) in nonterminal_definitions:
            self.__nonterminal_production_rules.append(self.__parse_production_rule(nonterminal_production_rule_tokens))

        return;

    def find_shortest_nonterminal_production_rule_suffix_match(self, symbol_sequence) -> NonTerminalSymbolNode:
        # TODO: figure out why/how to do this
        for i in range(1, len(symbol_sequence)):
            for nonterminal_definition in grammar:
                for grammar_rule in nonterminal_definition[1]:
                    if (is_matching(stack[-i:], grammar_rule[:i])):
                        return True
        return;

    def find_longest_nonterminal_production_rule_suffix_match(self, symbol_sequence) -> NonTerminalSymbolNode:
        longest_suffix_matching_nonterminal_prod_rule_candidates = [(nonterminal_id, [(end_node, []) for end_node in production_rule_graph[1]]) for nonterminal_id, production_rule_graph in enumerate(self.__nonterminal_production_rules)]
        longest_matched_nonterminal = None
        for i in range(1, len(symbol_sequence)):
            current_symbol = symbol_sequence[-i]
            new_candidates = []
            for nonterminal_id, intermediate_production_rule_nodes in longest_suffix_matching_nonterminal_prod_rule_candidates:
                next_intermediate_production_rule_nodes = []
                for intermediate_node, progress in intermediate_production_rule_nodes:
                    is_match = (isinstance(current_symbol, NonTerminalSymbolNode) and intermediate_node.is_match(current_symbol.nonterminal_id)) or intermediate_node.is_match(current_symbol)
                    if (is_match):
                        progress.append(current_symbol)
                        if (not intermediate_node.is_left_endpoint()):
                            # This intermediate node still has connections to the left. We add its
                            # left nodes as candidates for matching the next symbol.
                            next_intermediate_production_rule_nodes.extend((left_node, progress) for left_node in intermediate_node.left_nodes())
                        else:
                            # We are done progressing along this chain of nodes. This nonterminal's
                            # production rule has been fully matched, but it may not be the longest.
                            # This is a greedy matching algorithm, so we may still keep searching.
                            longest_matched_nonterminal = NonTerminalSymbolNode(nonterminal_id, self.__nonterminal_name_lookup[nonterminal_id], reversed(progress))
                if (len(next_intermediate_production_rule_nodes) > 0):
                    new_candidates.append((nonterminal_id, next_intermediate_production_rule_nodes))
            if (len(new_candidates) == 0):
                break;
            longest_suffix_matching_nonterminal_prod_rule_candidates = new_candidates
        return longest_matched_nonterminal
        

def reduce(stack : [SyntaxTreeNode], grammar):
    while (True):
        nonterminal_node = grammar.find_longest_nonterminal_production_rule_suffix_match(stack)
        if (not nonterminal_node):
            # No more reductions are possible. We have finished reducing.
            return;
        stack = stack[:-len(nonterminal_node.children())]
        stack.append(nonterminal_node)
    return;

def shift_reduce_parse_shader_code_syntax(shader_code : str, grammar):
    tokenized_shader_code = shader_code.split()
    stack = [tokenized_shader_code[0]]
    shift_offset = 1
    while(shift_offset < len(tokenized_shader_code))
        lookahead = tokenized_shader_code[shift_offset]
        if (not grammar.find_shortest_nonterminal_production_rule_suffix_match(stack + [lookahead])):
            # shift
            stack.append(lookahead)
            shift_offset += 1
        else:
            # reduce
            reduce(stack, grammar)

    assert len(stack) == 1
    return stack[0]

def get_shader_code_between_comment_markings(shader_code):

    return None

def parse_rendering_pipeline_uniforms_and_vertex_attributes(rendering_pipeline_path : str, grammar : EBNFGrammar):
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

        syntax_tree = shift_reduce_parse_shader_code_syntax(get_shader_code_between_comment_markings(shader_stage_code), grammar)
        
                    
def find_all_filepaths_in_directory_with_extension(root_path, extension, recursive=True):
    filepaths = []
    if(recursive):
        #TODO
    else:
        #TODO
    return None
        
def generate_cpp_structs_from_rendering_pipelines(engine_resources_path, game_resources_path):
    grammar = EBNFGrammar("./shader_language_grammar.txt")
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
            
        
