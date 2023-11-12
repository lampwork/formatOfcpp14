# formatOfcpp14
a toy of format , just cpp14 needed
```c++
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
```
