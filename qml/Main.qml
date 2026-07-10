import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1200
    height: 760
    visible: true
    title: "Skybound Tactics - 平台动作原型"
    color: "black"

    Rectangle {
        id: playField
        anchors.fill: parent
        color: "black"
        focus: true
        clip: true

        property bool aimingUp: false
        property bool aimingDown: false

        function attackDirection() {
            if (aimingUp) {
                return "up"
            }
            if (aimingDown) {
                return "down"
            }
            const chars = gameWorld.characters
            for (let i = 0; i < chars.length; ++i) {
                if (chars[i].id === "player") {
                    return chars[i].facingLeft ? "left" : "right"
                }
            }
            return "left"
        }

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
                aimingUp = true
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_S) {
                aimingDown = true
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
                aimingUp = false
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_S) {
                aimingDown = false
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
                source: resourceManager.image(modelData.imageKey)
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
                visible: modelData.kind !== "ground"
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

            Image {
                id: characterSprite
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height
                z: 2
                source: resourceManager.animationFrame(modelData.animationKey, modelData.frameIndex)
                fillMode: Image.PreserveAspectFit
                smooth: false

                transform: Scale {
                    origin.x: characterSprite.width / 2
                    origin.y: characterSprite.height / 2
                    xScale: modelData.facingLeft ? 1 : -1
                    yScale: 1
                }
            }
        }

        Repeater {
            model: gameWorld.characters

            Item {
                readonly property real vfxSize: modelData.attackVfxSize > 0 ? modelData.attackVfxSize : Math.max(modelData.attackBox.width, modelData.attackBox.height)

                x: modelData.attackBox.x + (modelData.attackBox.width - vfxSize) / 2
                y: modelData.attackBox.y + (modelData.attackBox.height - vfxSize) / 2
                width: vfxSize
                height: vfxSize
                z: 3
                visible: modelData.kind === "player" && (modelData.state === "attack" || modelData.state === "skill")
                clip: true

                Image {
                    x: -modelData.frameIndex * parent.vfxSize
                    y: 0
                    width: parent.vfxSize * 5
                    height: parent.vfxSize
                    source: "qrc:/resources/player/vfx_attack_" + modelData.attackDirection + ".png"
                    fillMode: Image.Stretch
                    smooth: false
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
                color: "transparent"
                border.color: "white"
                border.width: 2
                opacity: modelData.boxType === "attackBox" ? 0.55 : 0.9
                z: 20
            }
        }

        Text {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 16
            text: "A/D 移动 · Space 跳跃 · Shift 翻滚 · 右键攻击 · W/S+右键 上/下攻击"
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

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: function(mouse) {
                playField.forceActiveFocus()
                if (mouse.button === Qt.RightButton) {
                    gameWorld.playerAttack(playField.attackDirection())
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
