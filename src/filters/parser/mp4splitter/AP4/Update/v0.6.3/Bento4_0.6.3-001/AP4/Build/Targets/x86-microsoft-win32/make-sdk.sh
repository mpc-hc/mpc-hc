AP4_ROOT=../../..
SOURCE_ROOT=$AP4_ROOT/Source
BUILD_TARGET_DIR=$AP4_ROOT/Build/Targets/x86-microsoft-win32

CP="cp"
MKDIR="mkdir -p"
for config in Debug Release
do
    SDK_DIR=$config/SDK
    $MKDIR $SDK_DIR
    $MKDIR $SDK_DIR/include
    $MKDIR $SDK_DIR/bin
    $MKDIR $SDK_DIR/lib
    $CP $SOURCE_ROOT/Config/*.h $SDK_DIR/include
    $CP $SOURCE_ROOT/Core/*.h $SDK_DIR/include
    $CP $SOURCE_ROOT/Codecs/*.h $SDK_DIR/include
    $CP $SOURCE_ROOT/Crypto/*.h $SDK_DIR/include
    $CP $BUILD_TARGET_DIR/AP4/$config/AP4.lib $SDK_DIR/lib
    $CP $BUILD_TARGET_DIR/*/$config/*.exe $SDK_DIR/bin
done
