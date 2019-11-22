globalThis.Project = {
    selectView(view) {
        webkit.messageHandlers.selectView.postMessage(view);
    },

    pasteCode(code) {
        webkit.messageHandlers.pasteCode.postMessage(code);
    },

    openFile(file) {
        webkit.messageHandlers.openFile.postMessage(file);
    },

    openUrl(url) {
        webkit.messageHandlers.openUrl.postMessage(url);
    },

    askInterfaceForLocalVersion(author, title, compareWith) {
        if (typeof EXTENSIONS[author] === 'undefined' ||
            typeof EXTENSIONS[author][title] === 'undefined')
            return '';
        const { version, builtin } = EXTENSIONS[author][title];
        if (builtin)
            return '!';
        /* convert from internal representation to expected version string */
        const compareVersion = version === '' ? 'Version 1' : `Version ${version}`;
        const comparison = compareVersion > compareWith;
        if (comparison < 0)
            return '<';
        if (comparison > 0)
            return '>';
        return '=';
    },

    askInterfaceForLocalVersionText(author, title) {
        if (typeof EXTENSIONS[author] === 'undefined' ||
            typeof EXTENSIONS[author][title] === 'undefined')
            return undefined;
        const { version } = EXTENSIONS[author][title];
        return version === '' ? 'Version 1' : `Version ${version}`;
    },

    downloadMultipleExtensions(array) {
        webkit.messageHandlers.downloadMultipleExtensions.postMessage(array);
    },
};
