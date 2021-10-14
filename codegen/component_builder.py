import os

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
            
        
