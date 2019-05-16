(function () {
    var tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
    this.clickDownOnEntity = function (entityID, mouseEvent) {
        var runningSimplified = false;
        var scripts = ScriptDiscoveryService.getRunning();
        for (var i = 0; i < scripts.length; ++i) {
            if (scripts[i].name == "simplifiedUI.js") {
                runningSimplified = true;
                break;
            }
        }

        var avatarAppQML = runningSimplified ? "hifi/simplifiedUI/avatarApp/AvatarApp.qml" : "hifi/AvatarApp.qml";
        tablet.loadQMLSource(avatarAppQML);
    };
}
);
