#!/bin/bash

$ANDROID_NDK/ndk-build -j 3

if [ ! -d assets ]; then
	mkdir assets
fi
rm -rf assets\*.*

cp ../../../../bin/KlayGE.cfg assets
cp ../../../../media/Fonts/gkai00mp.kfont assets
cp ../../../../media/RenderFX/Blur.kfx assets
cp ../../../../media/RenderFX/Copy.kfx assets
cp ../../../../media/RenderFX/Font.kfx assets
cp ../../../../media/RenderFX/LensEffects.kfx assets
cp ../../../../media/RenderFX/PostToneMapping.kfx assets
cp ../../../../media/RenderFX/Resizer.kfx assets
cp ../../../../media/RenderFX/ToneMapping.kfx assets
cp ../../../../media/RenderFX/UI.kfx assets
cp ../../../../media/PostProcessors/Copy.ppml assets
cp ../../../../media/PostProcessors/LensEffects.ppml assets
cp ../../../../media/PostProcessors/PostToneMapping.ppml assets
cp ../../../../media/PostProcessors/Resizer.ppml assets
cp ../../../../media/Textures/2D/powered_by_klayge.dds assets
cp ../../../../media/Textures/2D/ui.dds assets
cp ../../../../media/Textures/3D/color_grading.dds assets
cp ../../../media/Text/text.txt assets
cp ../../../media/Text/Text.uiml assets

ant debug
adb install -r bin/KlayGE_Text-debug.apk
