# kapc_injector
kernel to user mode APC injector

![kapc_injector_banner_final_final_final](https://user-images.githubusercontent.com/90290279/159130409-b2e38287-3373-425e-b952-044fe6632b65.PNG)


# Requirements
- Visual Studio 2019 with latest Windows WDK and relevant packages.
- Windbg for kernel mode Debugging



# How to Use 
- 1. Compile kapc_injector driver and load kapc_injector driver by creating its service.

- 2. Compile kapc_injector_user & pass shellcode binary file path & process name to inject into as parameters

# Tests
- tested on Windows 10 1909

# References
https://wikileaks.org/ciav7p1/cms/page_7995519.html





