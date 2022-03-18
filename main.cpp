#include <iostream>
#include <vector>
#include <emscripten/bind.h>
#include <emscripten.h>
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
    std::vector<unsigned char> mem = {0};
    std::vector<int> inputBuffer;
    std::string tempInputBuffer;
    std::string output = "";
    int ip = 0;
    int cell_index = 0;
    int codeLength = code.length();
    int* brackets{ new int[codeLength]{} };
    std::vector<int> stack;
    while (ip < codeLength) {
        instruction = code.at(ip);
        if (instruction == "[") {
            stack.push_back(ip);
            
        } else if (instruction == "]") {
            brackets[ip] = stack.back();
            brackets[stack.back()] = ip;
            stack.pop_back();
        }
        ip++;
    }

    ip = 0;
    int toAdd = 1;
    std::string nextInstructionString;

    while (ip < codeLength) {
        instruction = code.at(ip);
        if (instruction == "+") {
            toAdd = 1;
            nextInstructionString = code.at(ip+1);
            if (nextInstructionString == "+") {
                ip++;
                while (nextInstructionString == "+") {
                    toAdd++;
                    ip++;
                    nextInstructionString = code.at(ip);
                }
                ip--;
            }
            mem[cell_index] = mem[cell_index] + toAdd;
            /*if (cell_index == 0) {
                consoleLog(std::to_string(ip)+": "+std::to_string(toAdd));
            } */
            
        } else if (instruction == "-") {
            toAdd = -1;
            nextInstructionString = code.at(ip+1);
            if (nextInstructionString == "-") {
                ip++;
                while (nextInstructionString == "-") {
                    toAdd--;
                    ip++;
                    nextInstructionString = code.at(ip+1);
                }
                
                ip--;
            }
            mem[cell_index] = mem[cell_index] + toAdd; 
        } else if (instruction == ">") {
            cell_index++;
            if (cell_index == mem.size()) {
                mem.push_back(0);
            }
        } else if (instruction == "<") {
            cell_index--;
        } else if (instruction == ".") {
            std::string str1(1, mem[cell_index]);
            //consoleLog(str1);
            output = output + str1;
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
                ip = brackets[ip];
            } else {
                std::string subString = code.substr(ip+2);
                if (subString == "[-]") {
                    mem[cell_index] = 0;
                    ip = brackets[ip];
                }
            }
        } else if (instruction == "]") {
            if (mem[cell_index] != 0) {
               ip = brackets[ip];
               
            };
        }
        ip++;
    }
    delete[] brackets;
    consoleLog("Done!");
    consoleLog(output);
}

EMSCRIPTEN_BINDINGS(my_module) {
    function("setCode", &setCode);
    function("execute", &execute);
}