#include <termios.h>
#include <unistd.h>

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

#include "src/consts.hpp"
#include "src/utils.hpp"
#include "src/cli.hpp"

#define KEY_UP 65
#define KEY_DOWN 66

namespace fs = std::filesystem;
using json = nlohmann::json;

/*
         (
          )
     __..---..__
 ,-='  /  |  \  `=-.
:--..___________..--;
 \.,_____________,./

*/

json collectProjectSettings() {
    json j;

    // Styling
    int style = askMulti("Which language for styling?", {"Css", "Scss"});
    j["style"] = (style == 0) ? "css" : "scss";

    int darkMode = askMulti("Do you want to implement auto dark-mode?", {"Yes", "No"});
    j["darkMode"] = (darkMode == 0) ? "true" : "false";

    // Scripting
    int script = askMulti("Which language for scripting?", {"JavaScript", "TypeScript"});
    j["script"] = (script == 0) ? "javascript" : "typescript";

    // Default
    j["lang"] = "en";
    j["imageOptimization"] = "lossy";
    j["fontOptimization"] = "true";

    return j;
}

void createProjectFiles(const std::string& name, const json& j) {
    // Create project.json and dump json
    write_file(name + "/config.json", j.dump(4));

    // Create head.html
    write_file(name + "/head.html", baseHead);

    // Create pages/ and default index.html
    fs::create_directory(name + "/pages");
    write_file(name + "/pages/index.html", "<h1>Welcome to the home page!</h1>");

    // Create style dir and default main
    std::string styleExt = (j["style"] == "css") ? "css" : "scss";
    std::string stylePath = name + "/" + styleExt;
    fs::create_directory(stylePath);
    write_file(stylePath + "/main." + styleExt, "");

    // Create script dir and default main
    std::string scriptExt = (j["script"] == "typescript") ? "ts" : "js";
    std::string scriptPath = name + "/" + scriptExt;
    fs::create_directory(name + "/" + scriptExt);
    write_file(scriptPath + "/main." + scriptExt, baseScript);

    // Create res/
    fs::create_directory(name + "/res");
}

/*
          _____                              __
         / ___/__  __ _  __ _  ___ ____  ___/ /__
        / /__/ _ \/  ' \/  ' \/ _ `/ _ \/ _  (_-<
        \___/\___/_/_/_/_/_/_/\_,_/_//_/\_,_/___/
*/

void help() {
    std::cout << bakerAscii <<
        // Help menu
        "\n"
        "\x1b[33m   baker\x1b[0m \x1b[30m[\x1b[35mcommand\x1b[30m] \x1b[30m[\x1b[35mflag\x1b[30m]\x1b[0m\n"
        "\n"
        "\x1b[0;1;34m build  \x1b[0;2m Bake the project\n"
        "\x1b[0m  -pretty\x1b[0;2m Without minimify\n"
        "\x1b[0;1;34m docs   \x1b[0;2m Open docs in your browser\n"
        "\x1b[0;1;34m init   \x1b[0;2m Create a new project\n"
        "\x1b[0m  -fast  \x1b[0;2m Without custom configs\n"
        "\x1b[0m  -sample\x1b[0;2m Template project\n"
        "\x1b[0;1;34m config \x1b[0;2m Configure your project\n"
        "\x1b[0;1;34m help   \x1b[0;2m Display this menu\n"
        "\x1b[0m\n";
}

void build(std::string path) {
    if (path.empty()) {
        // Without path
        if (fs::exists("config.json")) {
            // In root
            path = "./";
        } else {
            // Not in root, ask
            std::vector<std::string> dirs;

            for (const auto& entry : fs::directory_iterator("./")) {
                if (entry.is_directory()) {
                    std::string _path = entry.path().string();
                    if (fs::exists(_path + "/config.json"))
                        dirs.push_back(entry.path().string().substr(2));
                }
            }

            int dirsNum = dirs.size();
            if (dirsNum == 0) {
                error("No project found");
                return;
            } else if (dirsNum == 1) {
                path = dirs[0];
                info("Project " + path + " taken by default");
            } else {
                path = "./" + askMulti("Choose project path:", dirs);
            }
        }
    } else {
        // With path
        if (fs::exists(path + "/config.json")) {
            // Correct path
        } else {
            // Incorrect path
            error("Incorrect path");
            return;
        }
    }

    // Get config
    json j = json::parse(read_file(path + "/config.json"));

    // todo: optimize assets
    // todo: get head
    // todo: build pages
    //        - get content
    //        - implement head
    //        - implement footer if exists
    //        - minify if not -pretty
}

void docs() {
    int result = system("xdg-open https://github.com/ImSumire/Baker > /dev/null 2>&1 &");
    info("Docs opened");
}

void init() {
    bool overriding = false;

    // Asking project name
    std::string name = ask("Enter your project name: ");

    if (fs::exists(name)) {
        // Ask override
        overriding = askMulti("Are you sure you want to override the project named \x1b[4m" + name + "\x1b[0m?", {"No", "Yes"}) == 1;

        if (!overriding) {
            // Quit
            info("Ok bye");
            return;
        }
    }

    // Collecting project settings
    json j = collectProjectSettings();
    bool swup = !askMulti("Do you want to implement Swup?", {"Yes", "No"});
    bool defaultColors = !askMulti("Do you want default colors?", {"Yes", "No"});
    std::cout << std::endl;

    // Start initialization with settings
    info("Initializing " + name);
    auto start = std::chrono::system_clock::now();

    // Remove past project if overriding
    if (overriding) {
        fs::remove_all(name);
    }

    // Create project root and default files
    fs::create_directories(name);
    createProjectFiles(name, j);

    // Logging that project is initialized
    auto elapsed = getElapsed(start);
    success("Your project has been initialized in " + std::to_string(elapsed) + "ms!");
}

void config() {
    std::string lang;
    switch (askMulti("Which website lang you want?", {"English", "French", "Chinese", "Display all langs"})) {
        case 0:
            lang = "en";
            break;

        case 1:
            lang = "fr";
            break;

        case 2:
            lang = "ch";
            break;

        case 3:
            lang = askAllLangs();
            break;
    }

    int imageOptimization = askMulti("At what level will images be compressed?", {"Lossless", "Lossy", "Ultra"});
    int fontOptimization = askMulti("Will the fonts be optimized?", {"Yes", "No"});

    // todo: write config
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        help();
        return 0;
    }

    switch (hash(argv[1])) {
        case hash("build"):
        case hash("b"):
            if (argc > 2) {
                build(argv[3]);
            } else {
                build("");
            }
            break;

        case hash("docs"):
        case hash("d"):
            docs();
            break;

        case hash("init"):
        case hash("i"):
            init();
            break;

        case hash("config"):
        case hash("c"):
            config();
            break;

        default:
            help();
            break;
    }

    return 0;
}
