(function () {

    // 防止重复注入
    if (window.__bridge_injected__) {
        return;
    }

    window.__bridge_injected__ = true;

    // 检查 nativeBridge
    if (!window.nativeBridge) {
        console.log('nativeBridge not found');
        return;
    }

    /**
     * 创建 bridge
     * namespace = window.xxx
     */
    function createBridge(namespace) {

        return {

            postMessage: function (key, data) {

                window.nativeBridge.postMessage(
                    JSON.stringify({
                        namespace: namespace,
                        cmd: 'postMessage',
                        key: key,
                        data: data
                    })
                );
            },

            onEvent: function (json) {

                window.nativeBridge.postMessage(
                    JSON.stringify({
                        namespace: namespace,
                        cmd: 'onEvent',
                        data: json
                    })
                );
            },

            bgBiEvent: function (key, data) {

                window.nativeBridge.postMessage(
                    JSON.stringify({
                        namespace: namespace,
                        cmd: 'bgBiEvent',
                        key: key,
                        data: data
                    })
                );
            },

            sendEventCustom: function (key, data) {

                window.nativeBridge.postMessage(
                    JSON.stringify({
                        namespace: namespace,
                        cmd: 'sendEventCustom',
                        key: key,
                        data: data
                    })
                );
            }
        };
    }

    /**
     * 注册 bridge
     */
    function registerBridge(namespace) {

        if (!namespace) {
            return;
        }

        // 已存在则不覆盖
        if (window[namespace]) {
            return;
        }

        window[namespace] = createBridge(namespace);

        console.log('bridge registered -> ' + namespace);
    }

    /**
     * 暴露工厂
     */
    window.bridgeFactory = {
        register: registerBridge
    };

    /**
     * 默认注册 android
     */
    registerBridge('android');

    console.log('bridge.js injected success');

})();