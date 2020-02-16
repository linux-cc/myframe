#include <stdio.h>
#include "util/string.h"
#include <string.h>

//#define P_STR(desc, str)    printf("%s: {%d|%d|%p|%s}\n", desc, str.refCount(), str.length(), str.data(), str.data())
#define P_STR(fmt, str, ...)    printf(fmt ": {%d|%d|%p|%s}\n", ##__VA_ARGS__, str.refCount(), str.length(), str.data(), str.data())

int main(int argc, char *argv[]) {
    const char *data = "hello world";
    int length = (int)strlen(data);
    util::String str1 = data;
    str1.erase(0, 0);
    P_STR("str1 = data", str1);
    util::String str2 = str1;
    P_STR("str2 = str1", str2);
    P_STR("str1 = data", str1);
    util::String str3 = str2;
    P_STR("str3 = str2", str3);
    P_STR("str1 = data", str1);
    P_STR("str2 = str1", str2);
    str2[0] = str1[1];
    printf("after str2[0] = str1[1]\n");
    P_STR("str1 = data", str1);
    P_STR("str2 = str1", str2);
    P_STR("str3 = str2", str3);
    util::String str4(data, 5);
    P_STR("str4(data, 5)", str4);
    for (int i = 0; i < length; i++) {
        util::String str5(str1, i, util::String::npos);
        P_STR("str5(str1, %d, %d)", str5, i, util::String::npos);
        util::String str6(str1, i, length - i - 1);
        P_STR("str6(str1, %d, %d)", str6, i, length - i - 1);
    }
    util::String str7;
    str7.append(str1).append(data).append(data[0]);
    P_STR("str7.append(%s).append(%s).append(%c)", str7, str1.data(), data, data[0]);
    for (int i = 0; i < str7.length(); i++) {
        util::String str8 = str7;
        P_STR("str8 = str7", str7);
        util::String str9 = str7;
        P_STR("str9 = str8", str7);
        str8.erase(i, i);
        P_STR("str8.erase(%d, %d)", str8, i, i);
        P_STR("str7", str7);
        str9.erase(i, str7.length() - i);
        P_STR("str9.erase(%d, %d)", str9, i, str7.length() - i);
        P_STR("str7", str7);
    }
    for (int i = 0; i < length; i++) {
        util::String str10 = str1;
        str10.replace(i, length - i - 1, str1);
        P_STR("str10.replace(%d, %d, %s)", str10, i, length - i - 1, str1.data());
        util::String str11 = str1;
        str11.replace(i, util::String::npos, str1);
        P_STR("str11.replace(%d, %d, %s)", str11, i, util::String::npos, str1.data());
    }

    return 0;
}