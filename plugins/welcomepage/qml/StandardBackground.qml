/* KDevelop
 *
 * Copyright 2011 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.2

GroupBox
{
    id: bg
    property Component tools: null
    property string pageIcon
    property real marginLeft: toolbar.x+toolbar.width
    property real margins: 5

    SystemPalette { id: pal }
    
    Item {
        id: toolbar

        //radius: 5
        //color: pal.window//Qt.rgba(0.8, 0.8, 0.8, 0.4)

        anchors {
            top: parent.top
            left: parent.left
            margins: parent.margins
        }
        width: toolsLoader.width + 2*toolsLoader.margins
        height: toolsLoader.height + 2*toolsLoader.margins
        Loader {
            id: toolsLoader

            property int margins: 20

            anchors {
                top: parent.top
                left: parent.left
                margins: toolsLoader.margins
            }
            sourceComponent: tools
        }
    }
    
    Image {
        id: theIcon
        anchors {
            bottom: parent.bottom
            left: parent.left
            margins: 5
        }
        source: bg.pageIcon !== "" ? "image://icon/" + bg.pageIcon : ""
        width: 64
        height: width
    }
}
