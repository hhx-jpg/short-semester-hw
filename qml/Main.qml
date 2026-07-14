import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1200
    height: 760
    minimumWidth: 1200
    maximumWidth: 1200
    minimumHeight: 760
    maximumHeight: 760
    visible: true
    title: "Skybound Tactics - 平台动作原型"
    color: "black"

    Rectangle {
        id: playField
        anchors.fill: parent
        color: "black"
        focus: true
        clip: true

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_A) {
                gameWorld.playerRun(-1)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_D) {
                gameWorld.playerRun(1)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_W) {
                gameWorld.setAimUpPressed(true)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_S) {
                gameWorld.setAimDownPressed(true)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Space && !event.isAutoRepeat) {
                gameWorld.playerJump()
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Shift && !event.isAutoRepeat) {
                gameWorld.playerRoll()
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_K && !event.isAutoRepeat) {
                gameWorld.playerTakeHit(10)
                event.accepted = true
                return
            }
        }

        Keys.onReleased: function(event) {
            if (event.isAutoRepeat) {
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_A) {
                gameWorld.playerStopRun(-1)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_D) {
                gameWorld.playerStopRun(1)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_W) {
                gameWorld.setAimUpPressed(false)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_S) {
                gameWorld.setAimDownPressed(false)
                event.accepted = true
                return
            }
        }

        onWidthChanged: gameWorld.setViewport(width, height)
        onHeightChanged: gameWorld.setViewport(width, height)

        Repeater {
            model: gameWorld.mapLayers

            Image {
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height
                source: modelData.source
                opacity: modelData.opacity
                fillMode: Image.Stretch
                smooth: false
            }
        }

        Repeater {
            model: gameWorld.terrain

            Rectangle {
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height
                visible: false
                color: "#33ffffff"
                border.color: "#99ffffff"
                border.width: 1
                z: 1
            }
        }

        Timer {
            interval: 16
            running: true
            repeat: true
            onTriggered: gameWorld.tick(interval)
        }

        Repeater {
            model: gameWorld.characters

            Item {
                id: characterItem
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height
                z: 2
                clip: true

                Item {
                    id: characterSprite
                    anchors.fill: parent
                    clip: modelData.spriteIsSheet

                    Image {
                        anchors.fill: parent
                        visible: !modelData.spriteIsSheet
                        source: modelData.spriteSource
                        fillMode: Image.PreserveAspectFit
                        smooth: false
                    }

                    Image {
                        readonly property int sheetFrameCount: Math.max(1, modelData.spriteFrameCount)
                        readonly property real sourceFrameWidth: Math.max(1, modelData.spriteFrameWidth)
                        readonly property real sourceFrameHeight: Math.max(1, modelData.spriteFrameHeight)

                        x: 0
                        y: 0
                        width: characterSprite.width
                        height: characterSprite.height
                        visible: modelData.spriteIsSheet
                        source: modelData.spriteSource
                        sourceClipRect: Qt.rect(
                            modelData.spriteFrameIndex * sourceFrameWidth,
                            0,
                            sourceFrameWidth,
                            sourceFrameHeight)
                        fillMode: Image.Stretch
                        smooth: false
                    }

                    transform: Scale {
                        origin.x: characterSprite.width / 2
                        origin.y: characterSprite.height / 2
                        xScale: modelData.facingLeft ? 1 : -1
                        yScale: 1
                    }
                }

            }
        }

        Repeater {
            model: gameWorld.characters

            Item {
                readonly property bool vfxIsSheet: modelData.vfxIsSheet
                readonly property real baseVfxSize: modelData.attackVfxSize > 0 ? modelData.attackVfxSize : Math.max(modelData.attackBox.width, modelData.attackBox.height)
                readonly property real vfxSize: modelData.vfxIsSheet ? baseVfxSize * 1.4 : baseVfxSize
                readonly property int vfxFrameCount: Math.max(1, modelData.vfxFrameCount)
                readonly property real sourceFrameWidth: Math.max(1, modelData.vfxFrameWidth)
                readonly property real sourceFrameHeight: Math.max(1, modelData.vfxFrameHeight)

                x: modelData.attackBox.x + (modelData.attackBox.width - vfxSize) / 2
                y: modelData.attackBox.y + (modelData.attackBox.height - vfxSize) / 2
                width: vfxSize
                height: vfxSize
                z: 3
                visible: (modelData.state === "attack" || modelData.state === "skill") && modelData.vfxSource !== ""
                clip: true

                Image {
                    x: 0
                    y: 0
                    width: parent.vfxSize
                    height: parent.vfxSize
                    source: modelData.vfxSource
                    sourceClipRect: parent.vfxIsSheet
                        ? Qt.rect(
                            modelData.vfxFrameIndex * parent.sourceFrameWidth,
                            0,
                            parent.sourceFrameWidth,
                            parent.sourceFrameHeight)
                        : Qt.rect(0, 0, 0, 0)
                    fillMode: Image.Stretch
                    smooth: false

                    transform: Scale {
                        origin.x: width / 2
                        origin.y: height / 2
                        xScale: 1
                        yScale: 1
                    }
                }
            }
        }

        Repeater {
            model: gameWorld.debugBoxes

            Rectangle {
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height
                visible: modelData.active
                color: modelData.boxType === "terrain" ? "#6600ff00" : "transparent"
                border.width: 2
                opacity: 0.9
                z: 20

                border.color: {
                    if (modelData.boxType === "terrain") return "#00ff00"
                    if (modelData.boxType === "attackBox") return "#ff4444"
                    return "#ffffff"
                }
            }
        }

        // 血量心形显示
        Item {
            x: 14
            y: 66
            width: 310
            height: 35
            z: 100

            Row {
                anchors.fill: parent
                spacing: 4

                Repeater {
                    model: gameWorld.playerHeartCount

                    Image {
                        width: 26
                        height: 26
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/resources/ui_heart.png"
                        sourceSize.width: 32
                        sourceSize.height: 32
                    }
                }
            }
        }

        // 蓄力条 - 画面左下角
        Item {
            x: 14
            y: parent.height - 40
            width: 140
            height: 16
            z: 100
            visible: gameWorld.chargeProgress > 0.0

            Rectangle {
                anchors.fill: parent
                color: "#88000000"
                radius: 3
                border.color: "#888888"
                border.width: 1
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: 2
                width: (parent.width - 4) * gameWorld.chargeProgress
                radius: 2
                color: gameWorld.chargeProgress >= 1.0 ? "#ff4444" : "#44aaff"
            }

            Text {
                anchors.centerIn: parent
                text: gameWorld.chargeProgress >= 1.0 ? "释放！" : "蓄力 " + Math.round(gameWorld.chargeProgress * 100) + "%"
                color: "white"
                font.pixelSize: 11
                font.bold: true
            }
        }

        Text {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 16
            anchors.topMargin: 28
            text: "A/D 移动 · Space 跳跃 · Shift 翻滚 · 右键攻击(长按蓄力) · W/S+右键 上/下攻击 · K 受击"
            color: "white"
            opacity: 0.75
            font.pixelSize: 18
            z: 30
        }

        Text {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 16
            text: "Damage: " + gameWorld.damageCount
            color: "white"
            opacity: 0.9
            font.pixelSize: 22
            font.bold: true
            z: 30
        }

        Text {
            id: coordText
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 8
            text: "X: 0  Y: 0"
            color: "#88ff88"
            opacity: 0.7
            font.pixelSize: 14
            font.family: "monospace"
            z: 30
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true
            onPositionChanged: function(mouse) {
                coordText.text = "X: " + Math.round(mouse.x) + "  Y: " + Math.round(mouse.y)
            }
            onPressed: function(mouse) {
                playField.forceActiveFocus()
                if (mouse.button === Qt.RightButton) {
                    gameWorld.setChargePressed(true)
                }
            }
            onReleased: function(mouse) {
                if (mouse.button === Qt.RightButton) {
                    gameWorld.releaseAttack()
                }
            }
        }

        Component.onCompleted: {
            gameWorld.reset()
            gameWorld.setViewport(width, height)
            forceActiveFocus()
        }
    }
}
