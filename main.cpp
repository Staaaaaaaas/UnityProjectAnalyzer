#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <map>
#include <vector>
#include "yaml-cpp/yaml.h"


void dumpNode(const std::string &id, std::map<std::string, YAML::Node> &context,
              std::ofstream &out, int depth=0, bool newline=true){
    const auto& transform = context[id].begin()->second;
    const auto gameObjectID = transform["m_GameObject"]["fileID"].Scalar();
    const auto gameObject = context[gameObjectID];
    if(newline)out << '\n';
    for(int i=0;i<depth;i++)out << "--";
    out << gameObject["GameObject"]["m_Name"].Scalar();
    const auto children = transform["m_Children"];
    for(const auto& it:children){
        dumpNode(it["fileID"].Scalar(), context, out, depth+1);
    }
}

void dumpScene(const std::string &in_path, const std::string &out_path, std::vector<std::string> &used_scripts){
    std::string buffer;
    std::string line;
    std::ifstream in(in_path);
    std::ofstream out(out_path);

    std::string curr_id;
    std::map<std::string, YAML::Node> nodes;

    while(std::getline(in, line)){
        if(line.substr(0, 3) == "---" && !buffer.empty()){
            nodes[curr_id] = YAML::Load(buffer);
            curr_id = line.substr(line.find('&') + 1);
            buffer.clear();
        }
        else buffer += line + '\n';
    }
    const auto& rootNode= YAML::Load(buffer);
    const auto& roots = rootNode["SceneRoots"]["m_Roots"];
    bool newline = false;
    for(const auto &it:roots){
        dumpNode(it["fileID"].Scalar(), nodes, out, 0, newline);
        newline = true;
    }

    for(const auto &it:nodes){
        const auto &mono_behaviour = it.second["MonoBehaviour"];
        if(mono_behaviour){
            used_scripts.push_back(mono_behaviour["m_Script"]["guid"].Scalar());
        }
    }
}

std::string get_guid(std::ifstream &in){
    std::stringstream buffer;
    buffer << in.rdbuf();

    const auto& node = YAML::Load(buffer);
    return node["guid"].Scalar();
}

int dump_all_scenes(const std::string &in_path, const std::string &out_path, std::vector<std::string> &used_scripts){
    std::cout << "Dumping Scene hierarchies...\n";
    try {
        for (const auto &dir_entry: std::filesystem::recursive_directory_iterator(in_path)) {
            const auto &path = dir_entry.path();
            if (path.extension() == ".unity") {
                std::cout << "Processing scene " << path.stem() << " ...\n";
                const std::filesystem::path out_file(path.filename().string() + ".dump");
                dumpScene(path.string(), (out_path / out_file).string(), used_scripts);
            }
        }
        return 0;
    }
    catch(std::exception &exception) {
        return 1;
    }
}

int dump_unused_scripts(const std::string &in_path, const std::string &out_path,
                        std::vector<std::string> &used_scripts){
    try {
        const std::filesystem::path outFile("UnusedScripts.csv");
        std::ofstream out(out_path / outFile);
        out << "RelativePath,GUID";
        for (const auto &dirEntry: std::filesystem::recursive_directory_iterator(in_path)) {
            const auto &path = dirEntry.path();
            if (path.extension() == ".meta" && path.stem().extension() == ".cs") {
                std::ifstream in(path.string());
                std::string guid = get_guid(in);
                if (std::find(used_scripts.begin(), used_scripts.end(), guid) == used_scripts.end()) {
                    std::string relative_path = relative(path, in_path).string();
                    out << '\n' << relative_path.substr(0, relative_path.size() - 5) << ',' << guid;
                }
            }
        }
        return 0;
    }
    catch(std::exception &exception){
        return 1;
    }
}
int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    if(argc == 2 && strcmp(argv[1], "-h") == 0){
        std::cout << "This tool takes a Unity Project directory ([unity_project_path])\n"
                  << "and dumps hierarchy of its scenes as well as some information about\n"
                  << "unused scripts in the project\n"
                  << "to a specified output folder ([output_folder_path])\n";
        std::cout << std::string(10, '-') << '\n';
        std::cout << "Usage:\n";
        std::cout << "tool [unity_project_path] [output_folder_path]\n";
        return 0;
    }
    if(argc != 3){
        std::cerr << "Error! Wrong number of arguments!\n";
        return 1;
    }

    const std::string in_path = argv[1];
    const std::string out_path = argv[2];

    std::vector<std::string> used_scripts;
    std::vector<std::pair<std::string, std::string>> all_scripts;

    std::filesystem::path in_dir(in_path);
    if(!exists(in_dir)){
        std::cerr << "Error! Invalid Unity Project path!\n";
        return 1;
    }
    std::filesystem::path out_dir(out_path);
    if(!exists(out_dir)) create_directory(out_dir);

    if(dump_all_scenes(in_path, out_path, used_scripts)){
        std::cerr << "Error while Dumping the Scenes!\n";
        return 1;
    }
    std::cout << "Done!\n";
    std::cout << "Dumping Unused scripts...\n";
    if(dump_unused_scripts(in_path, out_path, used_scripts)){
        std::cerr << "Error while dumping Unused scripts!\n";
        return 1;
    }
    std::cout << "Done!\n";
    return 0;
}
