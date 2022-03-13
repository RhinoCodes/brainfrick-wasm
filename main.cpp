#include <iostream>
#include <vector>
#include <map>
#include <emscripten/bind.h>
#include <emscripten.h>
#include <algorithm>
using namespace emscripten;

std::string code = ",.";

EM_JS(char*, getBuffer, (), {
  var jsString = inputBuffer;
  // 'jsString.length' would return the length of the string as UTF-16
  // units, but Emscripten C strings operate as UTF-8.
  var lengthBytes = lengthBytesUTF8(jsString)+1;
  var stringOnWasmHeap = _malloc(lengthBytes);
  stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
  inputBuffer = "";
  return stringOnWasmHeap;
});

EM_JS(void, call_js_agrs, (const char *title, int lentitle), {
    jsMethodAgrs(UTF8ToString(title, lentitle));
});

void setCode(std::string codex) {
    code = codex;
}

void consoleLog(std::string x) {
  call_js_agrs(x.c_str(), x.length());
}

void execute() {
    std::string instruction = "";
    std::vector<int> mem = {0};
    std::vector<int> inputBuffer;
    std::string tempInputBuffer;
    int ip = 0;
    int cell_index = 0;
    std::map<int, int> brackets;
    std::vector<int> stack = {};
    while (ip < code.length()) {
        instruction = code.at(ip);
        if (instruction == "[") {
            stack.push_back(ip);
        } else if (instruction == "]") {
            brackets.insert(std::pair<int, int>(ip,stack.back()));
            brackets.insert(std::pair<int, int>(stack.back(),ip));
            stack.pop_back();
        }
        ip++;
    }
    ip = 0;

    while (ip < code.length()) {
        instruction = code.at(ip);
        if (instruction == "+") {
            mem[cell_index] = mem[cell_index] + 1;
            if (mem[cell_index] == 256) {
                mem[cell_index] = 0;
            }
        } else if (instruction == "-") {
            mem[cell_index] = mem[cell_index] - 1;
            if (mem[cell_index] == -1) {
                mem[cell_index] = 255;
            } 
        } else if (instruction == ">") {
            cell_index++;
            if (cell_index == mem.size()) {
                mem.push_back(0);
            }
        } else if (instruction == "<") {
            cell_index--;
        } else if (instruction == ".") {
            std::string str1(1, char(mem[cell_index]));
            consoleLog(str1);
        } else if (instruction == ",") {
            if (inputBuffer.size() == 0) {
                int x = -1;
                
                while (x == -1) {
                    std::string buffer(getBuffer());
                    if (buffer != "") {
                        if (buffer == "EMPTY") {
                            inputBuffer.push_back(10);
                        } else {
                            inputBuffer.push_back(10);
                            for (int i=0; i < buffer.length(); i++) {
                                inputBuffer.push_back((int)buffer.at(i));
                            }
                        }
                        

                        x = 0;
                    }
                    
                    emscripten_sleep(5);
               }
            }
            mem[cell_index] = inputBuffer.back();
            inputBuffer.pop_back();
        } else if (instruction == "[") {
            if (mem[cell_index] == 0) {
                ip = brackets.at(ip)-1;
            }
        } else if (instruction == "]") {
            if (mem[cell_index] != 0) {
               ip = brackets.at(ip)-1;
            };
        }
        ip++;
    }
}

EMSCRIPTEN_BINDINGS(my_module) {
    function("setCode", &setCode);
    function("execute", &execute);
}