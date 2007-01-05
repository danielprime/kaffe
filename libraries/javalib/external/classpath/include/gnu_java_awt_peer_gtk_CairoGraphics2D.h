/* DO NOT EDIT THIS FILE - it is machine generated */

#ifndef __gnu_java_awt_peer_gtk_CairoGraphics2D__
#define __gnu_java_awt_peer_gtk_CairoGraphics2D__

#include <jni.h>

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT jlong JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_init (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_disposeNative (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_drawPixels (JNIEnv *env, jobject, jlong, jintArray, jint, jint, jint, jdoubleArray, jdouble, jint);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_setGradient (JNIEnv *env, jobject, jlong, jdouble, jdouble, jdouble, jdouble, jint, jint, jint, jint, jint, jint, jint, jint, jboolean);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_setPaintPixels (JNIEnv *env, jobject, jlong, jintArray, jint, jint, jint, jboolean);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoSetMatrix (JNIEnv *env, jobject, jlong, jdoubleArray);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoScale (JNIEnv *env, jobject, jlong, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoSetOperator (JNIEnv *env, jobject, jlong, jint);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoSetRGBAColor (JNIEnv *env, jobject, jlong, jdouble, jdouble, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoSetFillRule (JNIEnv *env, jobject, jlong, jint);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoSetLine (JNIEnv *env, jobject, jlong, jdouble, jint, jint, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoSetDash (JNIEnv *env, jobject, jlong, jdoubleArray, jint, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoDrawGlyphVector (JNIEnv *env, jobject, jlong, jobject, jfloat, jfloat, jint, jintArray, jfloatArray);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoSetFont (JNIEnv *env, jobject, jlong, jobject);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoRelCurveTo (JNIEnv *env, jobject, jlong, jdouble, jdouble, jdouble, jdouble, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoRectangle (JNIEnv *env, jobject, jlong, jdouble, jdouble, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoArc (JNIEnv *env, jobject, jlong, jdouble, jdouble, jdouble, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoSave (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoRestore (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoNewPath (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoClosePath (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoMoveTo (JNIEnv *env, jobject, jlong, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoRelMoveTo (JNIEnv *env, jobject, jlong, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoLineTo (JNIEnv *env, jobject, jlong, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoRelLineTo (JNIEnv *env, jobject, jlong, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoCurveTo (JNIEnv *env, jobject, jlong, jdouble, jdouble, jdouble, jdouble, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoStroke (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoFill (JNIEnv *env, jobject, jlong, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoClip (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoPreserveClip (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoResetClip (JNIEnv *env, jobject, jlong);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoDrawLine (JNIEnv *env, jobject, jlong, jdouble, jdouble, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoDrawRect (JNIEnv *env, jobject, jlong, jdouble, jdouble, jdouble, jdouble);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_gtk_CairoGraphics2D_cairoFillRect (JNIEnv *env, jobject, jlong, jdouble, jdouble, jdouble, jdouble);
#undef gnu_java_awt_peer_gtk_CairoGraphics2D_INTERPOLATION_NEAREST
#define gnu_java_awt_peer_gtk_CairoGraphics2D_INTERPOLATION_NEAREST 0L
#undef gnu_java_awt_peer_gtk_CairoGraphics2D_INTERPOLATION_BILINEAR
#define gnu_java_awt_peer_gtk_CairoGraphics2D_INTERPOLATION_BILINEAR 1L
#undef gnu_java_awt_peer_gtk_CairoGraphics2D_INTERPOLATION_BICUBIC
#define gnu_java_awt_peer_gtk_CairoGraphics2D_INTERPOLATION_BICUBIC 5L
#undef gnu_java_awt_peer_gtk_CairoGraphics2D_ALPHA_INTERPOLATION_SPEED
#define gnu_java_awt_peer_gtk_CairoGraphics2D_ALPHA_INTERPOLATION_SPEED 2L
#undef gnu_java_awt_peer_gtk_CairoGraphics2D_ALPHA_INTERPOLATION_QUALITY
#define gnu_java_awt_peer_gtk_CairoGraphics2D_ALPHA_INTERPOLATION_QUALITY 3L
#undef gnu_java_awt_peer_gtk_CairoGraphics2D_ALPHA_INTERPOLATION_DEFAULT
#define gnu_java_awt_peer_gtk_CairoGraphics2D_ALPHA_INTERPOLATION_DEFAULT 4L

#ifdef __cplusplus
}
#endif

#endif /* __gnu_java_awt_peer_gtk_CairoGraphics2D__ */
