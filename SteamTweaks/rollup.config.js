import typescript from '@rollup/plugin-typescript';

export default {
	input: 'src/Tweaks/Overlay/HideFPSCounter.ts',
	output: {
		file: 'dist/glossiTweaks.js',
        sourcemap: "inline",
        format: 'es',
	},
    plugins: [typescript()]
};