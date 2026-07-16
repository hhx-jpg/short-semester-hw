import QtQuick
import QtQuick.Controls
import QtMultimedia

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

    property bool testMode: false

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
            return gameWorld.playerFacingLeft ? "left" : "right"
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
            if (event.key === Qt.Key_K && !event.isAutoRepeat) {
                gameWorld.playerTakeHit(10)
                event.accepted = true
                return
            }
        }

        Keys.onReleased: function(event) { //键盘输入
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

        Repeater {              //地图背景层
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

        Repeater {              // 地形 - 石质地面纹理
            model: gameWorld.terrain

            Image {
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height
                source: "qrc:/resources/stone_stage.png"
                fillMode: Image.Tile
                z: 1
                smooth: false
            }
        }

        Timer {                 //时钟
            interval: 16
            running: true
            repeat: true
            onTriggered: gameWorld.tick(interval)
        }

        Repeater {              //角色（玩家 + NPC）
            model: gameWorld.characterModel

            Item {
                id: characterItem
                x: model.x
                y: model.y
                width: model.width
                height: model.height
                z: 2
                clip: true

                Item {
                    id: characterSprite
                    anchors.fill: parent
                    clip: model.spriteIsSheet

                    Image {
                        anchors.fill: parent
                        visible: !model.spriteIsSheet
                        source: model.spriteSource
                        fillMode: Image.PreserveAspectFit
                        smooth: false
                    }

                    Image {
                        readonly property int sheetFrameCount: Math.max(1, model.spriteFrameCount)
                        readonly property real sourceFrameWidth: Math.max(1, model.spriteFrameWidth)
                        readonly property real sourceFrameHeight: Math.max(1, model.spriteFrameHeight)

                        x: 0
                        y: 0
                        width: characterSprite.width
                        height: characterSprite.height
                        visible: model.spriteIsSheet
                        source: model.spriteSource
                        sourceClipRect: Qt.rect(
                            model.spriteFrameIndex * sourceFrameWidth,
                            0,
                            sourceFrameWidth,
                            sourceFrameHeight)
                        fillMode: Image.Stretch
                        smooth: false
                    }

                    transform: Scale {
                        origin.x: characterSprite.width / 2
                        origin.y: characterSprite.height / 2
                        xScale: model.facingLeft ? 1 : -1
                        yScale: 1
                    }
                }

            }
        }

        Repeater {              //攻击特效叠加层（剑气/爆气等）
            model: gameWorld.characterModel

            Item {
                readonly property bool isRollVfx: model.rollAttack
                readonly property bool beeAttackVfx: !isRollVfx && model.state === "attack" && model.kind === "enemy"
                readonly property real baseVfxSize: model.attackVfxSize > 0 ? model.attackVfxSize : Math.max(model.attackBoxWidth, model.attackBoxHeight)
                readonly property real vfxSize: beeAttackVfx ? baseVfxSize * 1.4 : baseVfxSize
                readonly property int vfxFrameCount: beeAttackVfx ? model.vfxFrameCount : 5

                // Roll attack: 贴合攻击红框（细长），普通攻击: 正方形居中
                x: isRollVfx ? model.attackBoxX : model.attackBoxX + (model.attackBoxWidth - vfxSize) / 2
                y: isRollVfx ? model.attackBoxY : model.attackBoxY + (model.attackBoxHeight - vfxSize) / 2
                width: isRollVfx ? model.attackBoxWidth : vfxSize
                height: isRollVfx ? model.attackBoxHeight : vfxSize
                z: 3
                visible: (model.state === "attack" || model.state === "skill") && (model.kind === "player" || beeAttackVfx)
                clip: true

                // 淡蓝色剑气（5帧精灵表），roll attack 时拉伸适配红框
                Image {
                    x: -Math.min(model.frameIndex, parent.vfxFrameCount - 1) * parent.width
                    y: 0
                    width: parent.width * parent.vfxFrameCount
                    height: parent.height
                    source: model.vfxSource
                    fillMode: Image.Stretch
                    smooth: false

                    transform: Scale {
                        origin.x: width / 2
                        origin.y: height / 2
                        xScale: parent.beeAttackVfx && model.attackDirection === "left" ? -1 : 1
                        yScale: 1
                    }
                }
            }
        }

        Repeater {              // 调试碰撞箱（绿/白/红框）
            model: gameWorld.debugBoxes

            Rectangle {
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height
                visible: modelData.active && root.testMode
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
            //UI元素
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
            visible: root.testMode
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
            visible: root.testMode
        }

        MouseArea {             //鼠标输入
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
                    var wasCharged = gameWorld.chargeProgress >= 1.0
                    if (wasCharged) {
                        gameWorld.playerBurstAttack()
                    } else {
                        gameWorld.playerAttack(playField.attackDirection())
                    }
                    gameWorld.setChargePressed(false)
                }
            }
        }

        Component.onCompleted: {
            gameWorld.setViewport(width, height)
            forceActiveFocus()
        }
    }

    // ── 开始界面 ──
    Rectangle {
        anchors.fill: parent
        color: "#cc000000"
        z: 200
        visible: gameWorld.gameState === "start"

        Column {
            anchors.centerIn: parent
            spacing: 40

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Skybound Tactics"
                color: "white"
                font.pixelSize: 48
                font.bold: true
            }

            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20

                Button {
                    text: "正式开始"
                    font.pixelSize: 22
                    implicitWidth: 220
                    implicitHeight: 50
                    onClicked: {
                        root.testMode = false
                        gameWorld.startGame()
                        forceActiveFocus()
                    }
                }

                Button {
                    text: "测试版"
                    font.pixelSize: 22
                    implicitWidth: 220
                    implicitHeight: 50
                    onClicked: {
                        root.testMode = true
                        gameWorld.startGame()
                        forceActiveFocus()
                    }
                }

                Button {
                    text: "退出游戏"
                    font.pixelSize: 22
                    implicitWidth: 220
                    implicitHeight: 50
                    contentItem: Text {
                        text: "退出游戏"
                        color: "black"
                        font.pixelSize: 22
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: Qt.quit()
                }
            }
        }
    }

    // ── 死亡界面 ──
    Rectangle {
        anchors.fill: parent
        color: "#aa000000"
        z: 200
        visible: gameWorld.gameState === "dead"

        Text {
            anchors.centerIn: parent
            text: "你死了"
            color: "#ff4444"
            font.pixelSize: 48
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top + parent.height * 0.55
            text: "造成伤害: " + gameWorld.damageCount
            color: "#ffaa44"
            font.pixelSize: 24
            font.bold: true
            visible: root.testMode
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top + parent.height * 0.6
            text: "点击重新开始"
            color: "#aaaaaa"
            font.pixelSize: 20
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                gameWorld.returnToStartMenu()
                forceActiveFocus()
            }
        }
    }

    // ── 通关界面 ──
    Rectangle {
        anchors.fill: parent
        color: "#aa000000"
        z: 200
        visible: gameWorld.gameState === "win"

        Text {
            anchors.centerIn: parent
            text: "恭喜通关！"
            color: "#44ff44"
            font.pixelSize: 48
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top + parent.height * 0.55
            text: "造成伤害: " + gameWorld.damageCount
            color: "#ffaa44"
            font.pixelSize: 24
            font.bold: true
            visible: root.testMode
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top + parent.height * 0.6
            text: "点击返回主菜单"
            color: "#aaaaaa"
            font.pixelSize: 20
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                gameWorld.returnToStartMenu()
                forceActiveFocus()
            }
        }
    }

    // ── 音效播放器 ──
    MediaPlayer {
        id: actionSound
        audioOutput: AudioOutput {
            volume: 0.85
        }
    }

    MediaPlayer {
        id: impactSound
        audioOutput: AudioOutput {
            volume: 0.9
        }
    }

    MediaPlayer {
        id: playerRunSound
        source: resourceManager.audio("player.run")
        loops: MediaPlayer.Infinite
        audioOutput: AudioOutput {
            volume: 0.45
        }
    }

    MediaPlayer {
        id: enemyRunSound
        source: resourceManager.audio("enemy.run")
        loops: MediaPlayer.Infinite
        audioOutput: AudioOutput {
            volume: 0.38
        }
    }

    function playSound(key) {
        if (key === "player.run.start") {
            if (playerRunSound.playbackState !== MediaPlayer.PlayingState)
                playerRunSound.play()
            return
        }
        if (key === "player.run.stop") {
            playerRunSound.stop()
            return
        }
        if (key === "enemy.run.start") {
            if (enemyRunSound.playbackState !== MediaPlayer.PlayingState)
                enemyRunSound.play()
            return
        }
        if (key === "enemy.run.stop") {
            enemyRunSound.stop()
            return
        }

        const source = resourceManager.audio(key)
        if (source === "")
            return

        const player = key.indexOf("hurt") >= 0 || key.indexOf("dead") >= 0
            ? impactSound
            : actionSound
        player.stop()
        player.source = source
        player.play()
    }

    Connections {
        target: gameWorld
        function onSoundRequested(key) {
            root.playSound(key)
        }
        function onGameStateChanged() {
            if (gameWorld.gameState !== "playing") {
                playerRunSound.stop()
                enemyRunSound.stop()
            }
        }
    }

    // ── BGM 播放器 ──
    MediaPlayer {
        id: bgmPlayer
        source: resourceManager.audio("bgm.factory")
        loops: MediaPlayer.Infinite
        audioOutput: AudioOutput {
            volume: 0.35
        }
    }

    Connections {
        target: gameWorld
        function onGameStateChanged() {
            if (gameWorld.gameState === "playing") {
                bgmPlayer.play()
            } else {
                bgmPlayer.stop()
            }
        }
    }
}
