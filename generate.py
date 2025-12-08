import os
import json

def determine_markdown_file(file_name):
    """只处理 Markdown 文件"""
    return os.path.splitext(file_name)[1].lower() == '.md'

def is_code_file(file_name):
    """判断是否为代码文件(只在文件列表中显示,不生成JSON)"""
    code_extensions = (
        '.cc', '.cpp', '.cxx', '.c',      # C/C++
        '.h', '.hpp', '.hxx',             # C/C++ headers
        '.py',                            # Python
        '.js', '.ts',                     # JavaScript/TypeScript
        '.java',                          # Java
        '.go',                            # Go
        '.rs',                            # Rust
        '.sh', '.bat', '.ps1',            # Shell scripts
        '.json', '.xml',                  # Config files
        '.yaml', '.yml',                  # YAML
        '.txt',                           # Text
    )
    return os.path.splitext(file_name)[1].lower() in code_extensions

def determine_supported_file(file_name):
    """判断文件是否应该显示在文件列表中"""
    return determine_markdown_file(file_name) or is_code_file(file_name)

def get_modify_time(file_path):
    return int(os.path.getmtime(file_path) + 0.5)

def split_file_name(file_name):
    return os.path.splitext(file_name)[0]

def get_title_and_abstract(file_path):
    """只为 Markdown 文件提取标题和摘要"""
    title = None
    abstract = None
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            for line in f.readlines():
                if title is not None:
                    line = line.strip()
                    if len(line) == 0:
                        pass
                    elif ord(line[0]) in (33, 35, 40, 45, 46, 58, 91):
                        pass
                    elif len(line) > 120:
                        abstract = line[:120] + '...'
                    else:
                        abstract = line
                if line[:2]=='# ':
                    title = line[2:].strip()
                if title is not None and abstract is not None:
                    break
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
    
    return title if title else "", abstract if abstract else ""

def mywalk(directory, append_pointer, header, copy_target):
    for current_path, sub_paths, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(current_path, file)
            if determine_supported_file(file):
                last_edit_time = get_modify_time(file_path)
                file_size = os.path.getsize(file_path)
                
                # 判断是 Markdown 还是代码文件
                if determine_markdown_file(file):
                    # Markdown 文件: 生成 JSON 并提取标题摘要
                    title, abstract = get_title_and_abstract(file_path)
                    append_pointer.append({
                        'name': file,
                        'path': f"{header}_{split_file_name(file).replace(' ','_')}.json",
                        'lastedittime': last_edit_time,
                        'title': title,
                        'abstract': abstract,
                        'size': file_size,
                    })
                    # 生成 JSON 文件
                    with open(file_path,'r',encoding='utf-8') as fp:
                        content = fp.read()
                    output_target = os.path.join(copy_target, f"{header}_{split_file_name(file).replace(' ','_')}.json")
                    with open(output_target,'w',encoding='utf-8') as fp:
                        fp.write(json.dumps({'data':content}))
                
                elif is_code_file(file):
                    # 代码文件: 只记录信息,不生成 JSON
                    append_pointer.append({
                        'name': file,
                        'path': '',  # 代码文件不需要 JSON 路径
                        'lastedittime': last_edit_time,
                        'title': file,
                        'abstract': 'Code file',
                        'size': file_size,
                    })
        
        for path in sub_paths:
            path_path = os.path.join(current_path, path)
            new_append_pointer = []
            append_pointer.append({
                'name': path,
                'path': '',
                'lastedittime': get_modify_time(path_path),
                'size': -1,
                'children': new_append_pointer,
            })
            mywalk(path_path, new_append_pointer, f"{header}_{path.replace(' ','_')}", copy_target)
        break

root = os.path.abspath('./blog')
copy_target = os.path.abspath('public')

# clean
protect_file_list = ('ICON.ico', 'index.html', 'musics.json', 'musiccovers')
for files in os.walk(copy_target):
    files = files[2]; break

for rm_file in files:
    if rm_file not in protect_file_list:
        os.remove(os.path.join(copy_target, rm_file))

# copy
root_struct = []
append_pointer = root_struct
mywalk(root, root_struct, 'desktop', copy_target)
with open(os.path.join(copy_target, 'map.json'),'w',encoding='utf-8') as f:
    f.write(json.dumps(root_struct))