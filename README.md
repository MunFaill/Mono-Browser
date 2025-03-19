# Mono-Browser
A very simple browser using WebKit with WebView and GTK programmed in c

# Compile

You need a C/C++ compiler, in this case we are using GCC (GNU Compiler Collection).

Use this shell code on terminal (Linux):

```shell
gcc browser.c -o browser `pkg-config --cflags --libs webkit2gtk-4.0 gtk+-3.0`
```
