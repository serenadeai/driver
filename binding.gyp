{
  "targets": [{
    "target_name": "serenade-driver",
    "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
    "sources": ["src/driver.cpp"],
    "include_dirs": [
      "<!@(node -p \"require('node-addon-api').include\")"
    ],
    "conditions": [
      ['OS=="mac"', {
        "sources": ["src/driver.cpp", "src/mac.cpp"],
        "link_settings": {
          "libraries": [
            "/System/Library/Frameworks/AppKit.framework",
            "/System/Library/Frameworks/Carbon.framework",
            "/System/Library/Frameworks/CoreGraphics.framework",
            "/System/Library/Frameworks/Foundation.framework"
          ]
        },
        "xcode_settings": {
          "OTHER_CFLAGS": ["-ObjC++"]
        }
      }],
      ['OS=="win"', {
        "sources": ["src/driver.cpp", "src/windows.cpp"],
      }],
      ['OS=="linux"', {
        "sources": ["src/driver.cpp", "src/linux.cpp"],
        "link_settings": {
          "libraries": ["-lX11", "-lXtst"]
        }
      }]
    ]
  }]
}
