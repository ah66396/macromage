import {app, BrowserWindow} from 'electron';
import path from 'path';

import { getIsDev } from './util/devTools.js';

app.on('ready', () => {
    const mainWindow = new BrowserWindow({
        show: false
    });
    mainWindow.once('ready-to-show', () => {
        mainWindow.show()
    })
    
    if (getIsDev() === true) {
        mainWindow.loadURL("http://localhost:8080")
    } else {
        mainWindow.loadFile(path.join(app.getAppPath(), '/dist-react/index.html'))
    }
});