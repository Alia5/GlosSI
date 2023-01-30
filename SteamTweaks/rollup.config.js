import typescript from '@rollup/plugin-typescript';
import { readdirSync, lstatSync } from 'fs';
import path from 'path';

const getFileListForDir = (dir) => {
    return readdirSync(dir).map((file) => {
        const absolute = path.resolve(dir, file);
        if (file.endsWith('.ts')) {
            return absolute;
        }
        if (lstatSync(absolute).isDirectory()) {
            return getFileListForDir(absolute)
        }
    }).flat(999);

}


const tsPluginConf = typescript({
    cacheDir: '.rollup.tscache'
});

export default [
    {
        input: 'src/GlosSITweaks.ts',
        output: {
            dir: 'dist',
            sourcemap: "inline",
            format: 'iife',
        },
        plugins: [tsPluginConf],
    },
    ...getFileListForDir('src/Tweaks').map((file) => {
        return {
            input: file,
            output: {
                file: file.replace('src', 'dist').replace(/\.ts$/, '.js'),
                sourcemap: "inline",
                format: 'iife',
            },
            plugins: [tsPluginConf],
        }
    })
];