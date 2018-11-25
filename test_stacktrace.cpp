#include "mg_stacktrace.h"
#include <map>

using namespace mg;

namespace Nu {

char Buf[1024];

template<typename Type>
struct Alpha
{
    struct Beta
    {
	void func() {
    PrintStacktrace(Buf, sizeof(Buf));
	}
	void func(Type) {
    PrintStacktrace(Buf, sizeof(Buf));
	}
    };
};

struct Gamma
{
    template <int N>
    void unroll(double d) {
	unroll<N-1>(d);
    }
};

template<>
void Gamma::unroll<0>(double) {
    PrintStacktrace(Buf, sizeof(Buf));
}

} // namespace Nu

int main()
{
    Nu::Alpha<int>::Beta().func(42);
    printf("%s\n", Nu::Buf);
    Nu::Alpha<const char*>::Beta().func("42");
    printf("%s\n", Nu::Buf);
    Nu::Alpha< Nu::Alpha< std::map<int, double> > >::Beta().func();
    printf("%s\n", Nu::Buf);
    Nu::Gamma().unroll<5>(42.0);
    printf("%s\n", Nu::Buf);

}