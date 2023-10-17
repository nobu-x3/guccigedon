#include <iostream>
#include <vector>

int main(){
    std::vector<int> vec;
    for(int i = 0; i < 11; ++i){
        vec.push_back(i);
    }
    for(auto& i : vec){
        std::cout << &i << std::endl;
    }
    return 0;
}
