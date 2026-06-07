package com.cyrus.example.jnibridge;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

public final class JNIBridge {

    private JNIBridge() {
    }

    private static native Object invoke(long ptr, Class<?> clazz, Method method, Object[] args);

    private static native void delete(long ptr);

    public static Object newInterfaceProxy(long ptr, Class<?>... interfaces) {
        if (ptr == 0 || interfaces == null || interfaces.length == 0) {
            throw new IllegalArgumentException("ptr and interfaces must be valid");
        }
        ClassLoader loader = interfaces[0].getClassLoader();
        return Proxy.newProxyInstance(loader, interfaces, new InterfaceProxy(ptr));
    }

    public static void disableInterfaceProxy(Object proxy) {
        if (proxy == null || !Proxy.isProxyClass(proxy.getClass())) {
            return;
        }
        InvocationHandler handler = Proxy.getInvocationHandler(proxy);
        if (handler instanceof InterfaceProxy) {
            ((InterfaceProxy) handler).disable();
        }
    }

    private static final class InterfaceProxy implements InvocationHandler {
        private final Object invocationLock = new Object();
        private long ptr;

        InterfaceProxy(long ptr) {
            this.ptr = ptr;
        }

        @Override
        public Object invoke(Object proxy, Method method, Object[] args) {
            if (method.getDeclaringClass() == Object.class) {
                String name = method.getName();
                if ("toString".equals(name)) {
                    return "JNIBridgeProxy(" + ptr + ")";
                }
                if ("hashCode".equals(name)) {
                    return System.identityHashCode(proxy);
                }
                if ("equals".equals(name)) {
                    return proxy == (args != null && args.length > 0 ? args[0] : null);
                }
            }

            synchronized (invocationLock) {
                if (ptr == 0) {
                    return defaultValue(method.getReturnType());
                }
                Object result = JNIBridge.invoke(ptr, method.getDeclaringClass(), method, args);
                return result != null ? result : defaultValue(method.getReturnType());
            }
        }

        @Override
        protected void finalize() throws Throwable {
            try {
                disable(true);
            } finally {
                super.finalize();
            }
        }

        void disable() {
            disable(false);
        }

        private void disable(boolean releaseNative) {
            synchronized (invocationLock) {
                long oldPtr = ptr;
                ptr = 0;
                if (releaseNative && oldPtr != 0) {
                    JNIBridge.delete(oldPtr);
                }
            }
        }

        private static Object defaultValue(Class<?> type) {
            if (type == Void.TYPE || !type.isPrimitive()) {
                return null;
            }
            if (type == Boolean.TYPE) {
                return false;
            }
            if (type == Character.TYPE) {
                return (char) 0;
            }
            if (type == Byte.TYPE) {
                return (byte) 0;
            }
            if (type == Short.TYPE) {
                return (short) 0;
            }
            if (type == Integer.TYPE) {
                return 0;
            }
            if (type == Long.TYPE) {
                return 0L;
            }
            if (type == Float.TYPE) {
                return 0f;
            }
            if (type == Double.TYPE) {
                return 0d;
            }
            return null;
        }
    }
}
