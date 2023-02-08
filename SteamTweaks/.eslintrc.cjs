module.exports = {
    extends: [
        'eslint:recommended',
        'plugin:@typescript-eslint/eslint-recommended',
        'plugin:@typescript-eslint/recommended',
        'plugin:@typescript-eslint/recommended-requiring-type-checking',
    ],
    env: {
        browser: true,
        node: false
    },
    plugins: [
        '@typescript-eslint',
        'no-null',
        'prefer-arrow',
        'import',
    ],
    parser: '@typescript-eslint/parser',  // Specifies the ESLint parser
    parserOptions: {
        ecmaVersion: 2022,  // Allows for the parsing of modern ECMAScript features
        sourceType: 'module',
        ecmaFeatures: {
            jsx: false,
        },
        project: ['./tsconfig.json'],
        tsconfigRootDir: __dirname
    },
    rules: {
        '@typescript-eslint/no-namespace': 'off',
        '@typescript-eslint/ban-types': 'error',
        '@typescript-eslint/adjacent-overload-signatures': 'error',
        '@typescript-eslint/array-type': 'error',
        '@typescript-eslint/consistent-type-definitions': ['error', 'interface'],
        '@typescript-eslint/no-inferrable-types': 'error',
        '@typescript-eslint/no-misused-new': 'error',
        '@typescript-eslint/no-this-alias': 'error',
        '@typescript-eslint/prefer-for-of': 'error',
        '@typescript-eslint/prefer-function-type': 'error',
        '@typescript-eslint/prefer-namespace-keyword': 'error',
        'no-inner-declarations': 'off', // we have es6blocked scoped functions.
        '@typescript-eslint/triple-slash-reference': 'error',
        '@typescript-eslint/type-annotation-spacing': 'error',
        '@typescript-eslint/unified-signatures': 'error',
        '@typescript-eslint/no-explicit-any': 'error',
        '@typescript-eslint/no-unused-vars': 'error',
        '@typescript-eslint/unbound-method': 'warn',
        '@typescript-eslint/semi': [
            'error',
            'always'
        ],
        '@typescript-eslint/quotes': [
            'warn',
            'single'
        ],
        '@typescript-eslint/member-delimiter-style': [
            'error',
            {
                'multiline': {
                    'delimiter': 'semi',
                    'requireLast': true
                },
                'singleline': {
                    'delimiter': 'semi',
                    'requireLast': false
                }
            }
        ],
        '@typescript-eslint/indent': [
            'warn',
            4,
            {
                'FunctionDeclaration': {
                    'parameters': 'first'
                },
                'FunctionExpression': {
                    'parameters': 'first'
                },
                'SwitchCase': 1
            }
        ],

        '@typescript-eslint/explicit-member-accessibility': [
            'error',
            {
                'accessibility': 'explicit'
            }
        ],
        '@typescript-eslint/no-use-before-define': ['error', { 'functions': false }],
        "@typescript-eslint/naming-convention": [
            "error",
            {
                "selector": "default",
                "format": ["camelCase", "PascalCase"]
            },
            {
                "selector": "variable",
                "format": ["camelCase", "UPPER_CASE"]
            },
            {
                "selector": "parameter",
                "format": ["camelCase"],
                "leadingUnderscore": "allow"
            },
            {
                "selector": "memberLike",
                "modifiers": ["private"],
                "format": ["camelCase"],
                "leadingUnderscore": "require"
            },
            {
                "selector": "typeLike",
                "format": ["PascalCase"]
            }
        ],
        'no-console': 'off',
        'no-return-await': 'error',
        'arrow-body-style': 'error',
        'arrow-parens': [
            'error',
            'always'
        ],
        'camelcase': ['warn', { "ignoreImports": true }],
        'comma-dangle': [
            'error',
            {
                'objects': 'never',
                'arrays': 'never',
                'functions': 'never'
            }
        ],
        'prefer-arrow/prefer-arrow-functions': 'error',
        'prefer-arrow-callback': 'error',
        'prefer-const': 'error',
        'quote-props': [
            'error',
            'consistent-as-needed'
        ],
        'no-var': 'error',
        'new-parens': 'error',
        'no-caller': 'error',
        'no-cond-assign': 'error',
        'no-debugger': 'error',
        'no-empty': 'error',
        'no-eval': 'error',
        'no-multiple-empty-lines': 'warn',
        'no-new-wrappers': 'error',
        'no-redeclare': 'error',
        'no-shadow': [
            'error',
            {
                'hoist': 'all'
            }
        ],
        'no-null/no-null': 'error',
        'no-throw-literal': 'error',
        'no-trailing-spaces': 'error',
        'no-undef-init': 'error',
        'no-underscore-dangle': 'error',
        'no-unsafe-finally': 'error',
        'no-unused-labels': 'error',
        'spaced-comment': 'error',
        'use-isnan': 'error',
        'max-lines': [
            'error',
            {
                'max': 300,
                'skipBlankLines': true,
                'skipComments': true
            }
        ],
        'max-len': [
            'warn',
            {
                'code': 140
            }
        ],
        'dot-notation': 'error',
        'eqeqeq': 'error',
        'eol-last': 'error',
        'linebreak-style': ['error', 'windows'],
        'block-spacing': ['error', 'always'],
        'object-curly-spacing': ["error", "always"],
        'import/no-deprecated': 'warn', // eslint deprecation rule sucks. just wrns on deprecated IMPORTs
    },
    settings: {
    },
};