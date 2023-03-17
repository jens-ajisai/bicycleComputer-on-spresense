SPRESENSE_SDK=__REPLACE_WITH_YOUR_PATH_TO_THE_SPRESENSE_SDK__

if [ "$SPRESENSE_SDK" == "__REPLACE_WITH_YOUR_PATH_TO_THE_SPRESENSE_SDK__" ]; then
    echo "Please adapt the variable SPRESENSE_SDK inside this script $0."
    exit 1
fi

CURRENT_DIR=`pwd`
echo $CURRENT_DIR

cd $SPRESENSE_SDK
git apply $CURRENT_DIR/../patches/spresense.patch

cd $SPRESENSE_SDK/sdk/apps
git apply $CURRENT_DIR/../patches/apps.patch

cd $SPRESENSE_SDK/nuttx
git apply $CURRENT_DIR/../patches/nuttx.patch

sed -i 's/__REPLACE_ME_WITH_YOUR_PROJECT_PATH__/$CURRENT_DIR\/../g' $SPRESENSE_SDK/nuttx/arch/arm/src/Makefile
