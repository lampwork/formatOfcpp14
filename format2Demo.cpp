#include"format2.hpp"

class Type {
    friend std::ostream &operator<<(std::ostream &o, const Type &) {
        return o << "Type";
    }
};

void MacroTest() {
    //print "{FF  ,FFF " ==> Hex, width=4, left aligned, fill ' '
    //使用宏定义
    FMTSS("%{{Hw4lf },{Hw4lf }\n", 0xff, 0xfff);
}

void CompatibilityTest() {
    Type type;
    FMT("{}\n", type);
}

void ErrorTest() {
    // FMT("{},{}", 1, 2, 3); //static_assert: args is too much
    // FMT("{},{}", 1); //static_assert: fmt is too much
}

/** note:
 * fmt::Fmt(CTS(fmt), args...)
 * 1. placeholder: {}
 * 2. Format character：
 *      h -- hex
 *      H -- Hex
 *      w(num) -- width=num
 *      l -- left aligned（if no 'r'，default to right aligned）
 *      f(char) -- fill char（e.g. f0 --> fill 0）
 * 3. print Hex, width=8, left aligned, fill with '_'
 *  use: fmt::Fmt("0x{Hw8lf_}", 0xff);
 *  get: "0x000000FF"
 * 4. use defined class: just need to complete operator<< for std::ostream&
 * 5. escape '%' ==> "%%"
 * 6. escape '{' ==> "%{"
*/
int main()
{
    DEF_CTS();
    //print "{0x00ff" ==> hex, width=4, right aligned, fill 0
    auto ss = fmt::Fmtss(CTS("%{0x{hw4f0}"), 0xff);
    std::cout << ss.str() << std::endl;
    MacroTest();
    CompatibilityTest();
}
