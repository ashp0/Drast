import {
	CompletionItem,
	CompletionItemKind,
	CompletionParams,
	createConnection,
	Diagnostic,
	DiagnosticSeverity,
	DocumentFormattingParams,
	DocumentRangeFormattingParams,
	Hover,
	InitializeParams,
	InitializeResult,
	InsertTextFormat,
	Location,
	MarkupKind,
	Position,
	ProposedFeatures,
	Range,
	SemanticTokens,
	SemanticTokensBuilder,
	SemanticTokensParams,
	SemanticTokensRangeParams,
	TextDocumentPositionParams,
	TextDocumentSyncKind,
	TextDocuments,
	TextEdit
} from 'vscode-languageserver/node';
import { TextDocument } from 'vscode-languageserver-textdocument';
import * as fs from 'fs/promises';
import * as syncFs from 'fs';
import * as path from 'path';
import { fileURLToPath, pathToFileURL } from 'url';

type SymbolKind =
	| 'function'
	| 'method'
	| 'variable'
	| 'parameter'
	| 'struct'
	| 'enum'
	| 'enumVariant'
	| 'protocol'
	| 'field'
	| 'typeParameter'
	| 'cppFunction'
	| 'cppType'
	| 'cppEnum'
	| 'cppVariable'
	| 'namespace'
	| 'builtin';

type SymbolScope = 'global' | 'member' | 'local' | 'import' | 'builtin';

interface DrastParameter {
	name: string;
	typeName?: string;
	defaultValue?: string;
}

interface DrastSymbol {
	name: string;
	qualifiedName?: string;
	kind: SymbolKind;
	scope: SymbolScope;
	completionKind: CompletionItemKind;
	uri: string;
	range: Range;
	selectionRange: Range;
	definitionLine: string;
	detail: string;
	signature: string;
	returnType?: string;
	typeName?: string;
	parameters?: DrastParameter[];
	containerName?: string;
	documentation?: string;
	insertText?: string;
	sortText?: string;
}

interface BlockContext {
	indent: number;
	kind: 'struct' | 'enum' | 'impl' | 'protocol' | 'function' | 'comptime' | 'control';
	name?: string;
	hostName?: string;
	line?: number;
}

interface CommentState {
	blockDepth: number;
}

interface UseImport {
	raw: string;
	pathText: string;
	isHeader: boolean;
	isQuoted: boolean;
	isFile: boolean;
	alias?: string;
}

interface FunctionHeader {
	name: string;
	displayName: string;
	typeParameters: string[];
	parameters: DrastParameter[];
	returnType: string;
	discard: boolean;
	visibility?: string;
	isOperator: boolean;
	operatorSymbol?: string;
	signature: string;
}

interface VariableDeclaration {
	name: string;
	character: number;
	detail: string;
	typeName?: string;
	signature: string;
}

interface CompletionContext {
	receiver?: string;
	shorthandVariant: boolean;
	useImport: boolean;
}

const connection = createConnection(ProposedFeatures.all);
const documents = new TextDocuments(TextDocument);

let workspaceRootUris: string[] = [];
let workspaceScanPromise: Promise<void> | undefined;
let hasScannedWorkspace = false;

const documentSymbols = new Map<string, DrastSymbol[]>();
const symbolTable = new Map<string, DrastSymbol[]>();
const memberTable = new Map<string, DrastSymbol[]>();
const enumVariantTable = new Map<string, DrastSymbol[]>();
const headerSymbolCache = new Map<string, DrastSymbol[]>();

const primitiveTypes = [
	'int',
	'Int',
	'float',
	'Float',
	'double',
	'Double',
	'bool',
	'Bool',
	'char',
	'Char',
	'string',
	'String',
	'usize'
];

const wordOperators = [
	'iseq',
	'isne',
	'islt',
	'isgt',
	'islteq',
	'isgteq',
	'islte',
	'isgte',
	'and',
	'or',
	'not'
];

const unsupportedWordOperators = ['shl', 'shr', 'bor', 'band', 'bxor'];

const semanticTokenTypes = [
	'function',
	'method',
	'parameter',
	'variable',
	'property',
	'enumMember',
	'type',
	'namespace'
] as const;

type SemanticTokenType = typeof semanticTokenTypes[number];

const semanticTokenTypeIndexes = new Map<SemanticTokenType, number>(
	semanticTokenTypes.map((tokenType, index) => [tokenType, index])
);

const keywordDescriptions: Record<string, string> = {
	use: 'Import a Drast module, enable the runtime, or emit a C++ include.',
	const: 'Marks a top-level global as C++ const.',
	return: 'Return from the current function.',
	if: 'Start a conditional branch.',
	elif: 'Continue a conditional branch with another condition.',
	else: 'Fallback branch for a conditional.',
	while: 'Loop while a condition is true.',
	for: 'Iterate over a collection or range.',
	break: 'Exit the nearest loop.',
	continue: 'Skip to the next loop iteration.',
	in: 'Separates a loop variable from its collection or range.',
	to: 'Creates an inclusive range end in a for loop.',
	until: 'Creates an exclusive range end in a for loop.',
	step: 'Sets the increment for a range loop.',
	struct: 'Declare a C++ struct-backed type.',
	enum: 'Declare a simple enum class or tagged data enum.',
	impl: 'Declare methods for a type.',
	protocol: 'Declare an abstract protocol interface.',
	match: 'Dispatch over values or data enum variants.',
	default: 'Fallback branch for a match.',
	with: 'Attach generic constraints after a function declaration.',
	as: 'Declare protocol conformance in an impl block.',
	try: 'Start a C++ try block.',
	catch: 'Handle exceptions from a try block.',
	private: 'Mark a struct field or method as private.',
	preview: 'Accepted field visibility marker emitted like public.',
	fileprivate: 'Accepted before a top-level struct or enum.',
	discard: 'Suppress nodiscard-style return warnings for a function.',
	operator: 'Declare an overloaded operator in an impl block.',
	maybe: 'Wrap a function return type in std::optional.',
	true: 'Boolean true literal.',
	false: 'Boolean false literal.',
	nil: 'Optional empty value literal.',
	self: 'Current method receiver.',
	variadic: 'Type constructor for std::initializer_list.',
	tuple: 'Type and expression marker for std::tuple.',
	comptime: 'Compile-time block marker used by newer Drast tooling.',
	throws: 'Function effect marker for fallible signatures in newer Drast tooling.',
	iseq: 'Word equality operator.',
	isne: 'Word inequality operator.',
	islt: 'Word less-than operator.',
	isgt: 'Word greater-than operator.',
	islteq: 'Word less-than-or-equal operator.',
	isgteq: 'Word greater-than-or-equal operator.',
	islte: 'Accepted comparison word for less-than-or-equal.',
	isgte: 'Accepted comparison word for greater-than-or-equal.',
	and: 'Logical and operator.',
	or: 'Logical or operator.',
	not: 'Logical not operator.',
	shl: 'Bitwise shift-left token. The current parser lexes it but does not accept it as an expression operator.',
	shr: 'Bitwise shift-right token. The current parser lexes it but does not accept it as an expression operator.',
	bor: 'Bitwise-or token. The current parser lexes it but does not accept it as an expression operator.',
	band: 'Bitwise-and token. The current parser lexes it but does not accept it as an expression operator.',
	bxor: 'Bitwise-xor token. The current parser lexes it but does not accept it as an expression operator.',
	nothing: 'Placeholder keyword for an intentionally empty block.'
};

const declarationKeywords = new Set([
	...Object.keys(keywordDescriptions),
	...unsupportedWordOperators
]);

const blockIntroducers = new Set([
	'if',
	'elif',
	'else',
	'while',
	'for',
	'struct',
	'enum',
	'impl',
	'protocol',
	'match',
	'default',
	'try',
	'catch',
	'comptime'
]);

const builtinSymbols: DrastSymbol[] = [
	builtin('print', CompletionItemKind.Function, 'print values;variadic[T]', 'void', 'Write values without a trailing newline.'),
	builtin('println', CompletionItemKind.Function, 'println values;variadic[T]', 'void', 'Write values followed by a newline.'),
	builtin('printf', CompletionItemKind.Function, 'printf format;string values;variadic[T]', 'void', 'Forward a formatted print call to the runtime.'),
	builtin('getInput', CompletionItemKind.Function, 'getInput prompt;string;""', 'string', 'Read a line from standard input.'),
	builtin('arg', CompletionItemKind.Function, 'arg index;usize', 'string', 'Read a command-line argument.'),
	builtin('args', CompletionItemKind.Function, 'args', '{string}', 'Return all command-line arguments.'),
	builtin('readFile', CompletionItemKind.Function, 'readFile path;string', 'string', 'Read the contents of a file.'),
	builtin('writeFile', CompletionItemKind.Function, 'writeFile path;string contents;string', 'bool', 'Write text to a file.'),
	builtin('fileExists', CompletionItemKind.Function, 'fileExists path;string', 'bool', 'Return whether a file exists.'),
	builtin('toString', CompletionItemKind.Function, 'toString value;T', 'string', 'Convert a value to a string.'),
	builtin('parseInt', CompletionItemKind.Function, 'parseInt text;string', 'maybe int', 'Parse an integer from text.'),
	builtin('parseFloat', CompletionItemKind.Function, 'parseFloat text;string', 'maybe double', 'Parse a floating-point value from text.'),
	builtin('charCode', CompletionItemKind.Function, 'charCode text;string', 'int', 'Return the character code of the first character.'),
	builtin('isAlpha', CompletionItemKind.Function, 'isAlpha text;string', 'bool', 'Return whether the first character is alphabetic.'),
	builtin('isDigit', CompletionItemKind.Function, 'isDigit text;string', 'bool', 'Return whether the first character is numeric.'),
	builtin('isAlnum', CompletionItemKind.Function, 'isAlnum text;string', 'bool', 'Return whether the first character is alphanumeric.'),
	builtin('isWhitespace', CompletionItemKind.Function, 'isWhitespace text;string', 'bool', 'Return whether the first character is whitespace.'),
	builtin('isUpper', CompletionItemKind.Function, 'isUpper text;string', 'bool', 'Return whether the first character is uppercase.'),
	builtin('isLower', CompletionItemKind.Function, 'isLower text;string', 'bool', 'Return whether the first character is lowercase.'),
	builtin('errorCount', CompletionItemKind.Function, 'errorCount', 'int', 'Return the number of structured runtime diagnostics.'),
	builtin('hasErrors', CompletionItemKind.Function, 'hasErrors', 'bool', 'Return whether structured runtime diagnostics exist.'),
	builtin('reportError', CompletionItemKind.Function, 'reportError file;string line;int column;int message;string', 'void', 'Record a structured runtime diagnostic.'),
	builtin('clearErrors', CompletionItemKind.Function, 'clearErrors', 'void', 'Clear structured runtime diagnostics.')
];

const snippetCompletions: CompletionItem[] = [
	snippet('main', 'main, int\n\t${1:return 0}', 'New Drast program entry point'),
	snippet('function', '${1:name} ${2:param};${3:int}, ${4:int}\n\t${5:return ${2}}', 'Function declaration'),
	snippet('void function', '${1:name} ${2:param};${3:int}\n\t${4:nothing}', 'Void function declaration'),
	snippet('struct', 'struct ${1:Name}\n\t${2:field} ${3:int}', 'Struct definition'),
	snippet('enum', 'enum ${1:Name}\n\t${2:First}; ${3:Second}; ${4:Third};', 'Simple enum definition'),
	snippet('data enum', 'enum ${1:Expr}\n\t${2:Case} ${3:value};${4:int}', 'Tagged data enum definition'),
	snippet('impl', 'impl ${1:Type}\n\t${2:method}, ${3:void}\n\t\t${4:nothing}', 'Impl block'),
	snippet('protocol', 'protocol ${1:Name}\n\t${2:method}, ${3:void}', 'Protocol declaration'),
	snippet('conformance', 'impl ${1:Type} as ${2:Protocol}\n\t${3:method}, ${4:void}\n\t\t${5:nothing}', 'Protocol conformance block'),
	snippet('match', 'match ${1:value}\n\t${2:Pattern}\n\t\t${3:nothing}\n\tdefault\n\t\t${4:nothing}', 'Match expression'),
	snippet('if', 'if ${1:condition}\n\t${2:nothing}', 'Conditional block'),
	snippet('for in', 'for ${1:item} in ${2:items}\n\t${3:nothing}', 'Foreach loop'),
	snippet('for range', 'for ${1:i} in ${2:0} until ${3:count}\n\t${4:nothing}', 'Range loop'),
	snippet('try catch', 'try\n\t${1:nothing}\ncatch ;${2:error}\n\t${3:println ${2}}', 'Try/catch block'),
	snippet('comptime', 'comptime\n\t${1:nothing}', 'Compile-time block'),
	snippet('operator', 'operator[${1:==}] ${2:left};${3:Type} ${4:right};${3:Type}, ${5:bool}\n\t${6:return ${2} iseq ${4}}', 'Operator overload'),
	snippet('nothing', 'nothing', 'Intentionally empty block')
];

connection.onInitialize((params: InitializeParams): InitializeResult => {
	workspaceRootUris = collectWorkspaceRoots(params);

	return {
		capabilities: {
			textDocumentSync: TextDocumentSyncKind.Incremental,
			completionProvider: {
				resolveProvider: false,
				triggerCharacters: ['.', '`', ';', ' ']
			},
			hoverProvider: true,
			definitionProvider: true,
			semanticTokensProvider: {
				legend: {
					tokenTypes: [...semanticTokenTypes],
					tokenModifiers: []
				},
				full: true,
				range: true
			},
			documentFormattingProvider: true,
			documentRangeFormattingProvider: false
		}
	};
});

connection.onInitialized(() => {
	void scheduleWorkspaceScan();
});

connection.onDidChangeWatchedFiles(() => {
	headerSymbolCache.clear();
	void scheduleWorkspaceScan();
});

documents.onDidOpen((event) => {
	updateDocumentSymbols(event.document);
});

documents.onDidChangeContent((event) => {
	updateDocumentSymbols(event.document);
});

documents.onDidSave((event) => {
	updateDocumentSymbols(event.document);
});

documents.onDidClose((event) => {
	connection.sendDiagnostics({ uri: event.document.uri, diagnostics: [] });
});

connection.onCompletion(async (params: CompletionParams): Promise<CompletionItem[]> => {
	await ensureWorkspaceScanned();

	const document = documents.get(params.textDocument.uri);
	if (!document) {
		return baseCompletionItems(params.textDocument.uri);
	}

	const context = completionContext(document, params.position);
	if (context.useImport) {
		return importCompletionItems();
	}

	if (context.receiver !== undefined || context.shorthandVariant) {
		return memberCompletionItems(document, params.position, context);
	}

	return [
		...snippetCompletions,
		...keywordCompletionItems(),
		...localCompletionItems(document, params.position),
		...symbolCompletionItems(params.textDocument.uri)
	];
});

connection.languages.semanticTokens.on(async (params: SemanticTokensParams): Promise<SemanticTokens> => {
	await ensureWorkspaceScanned();
	return semanticTokensForDocument(params.textDocument.uri);
});

connection.languages.semanticTokens.onRange(async (params: SemanticTokensRangeParams): Promise<SemanticTokens> => {
	await ensureWorkspaceScanned();
	return semanticTokensForDocument(params.textDocument.uri, params.range);
});

connection.onHover(async (params: TextDocumentPositionParams): Promise<Hover | null> => {
	await ensureWorkspaceScanned();

	const document = documents.get(params.textDocument.uri);
	if (!document) {
		return null;
	}

	const token = tokenAtPosition(document, params.position);
	if (!token) {
		return null;
	}

	const keywordDescription = keywordDescriptions[token.text];
	if (keywordDescription) {
		return {
			contents: {
				kind: MarkupKind.Markdown,
				value: `**${token.text}**\n\n${keywordDescription}`
			},
			range: token.range
		};
	}

	const symbol = resolveSymbolAtToken(document, params.position, token.text);
	if (!symbol) {
		return null;
	}

	return {
		contents: {
			kind: MarkupKind.Markdown,
			value: hoverMarkdown(symbol)
		},
		range: token.range
	};
});

connection.onDefinition(async (params: TextDocumentPositionParams): Promise<Location | null> => {
	await ensureWorkspaceScanned();

	const document = documents.get(params.textDocument.uri);
	if (!document) {
		return null;
	}

	const token = tokenAtPosition(document, params.position);
	if (!token || keywordDescriptions[token.text]) {
		return null;
	}

	const symbol = resolveSymbolAtToken(document, params.position, token.text);
	if (!symbol) {
		return null;
	}

	return Location.create(symbol.uri, symbol.selectionRange);
});

connection.onDocumentFormatting((_params: DocumentFormattingParams): TextEdit[] => {
	const document = documents.get(_params.textDocument.uri);
	if (!document) {
		return [];
	}

	return formatDocumentEdits(document);
});

connection.onDocumentRangeFormatting((_params: DocumentRangeFormattingParams): TextEdit[] => {
	return [];
});

documents.listen(connection);
connection.listen();

function snippet(label: string, insertText: string, detail: string): CompletionItem {
	return {
		label,
		kind: CompletionItemKind.Snippet,
		detail,
		insertText,
		insertTextFormat: InsertTextFormat.Snippet,
		documentation: {
			kind: MarkupKind.Markdown,
			value: `\`\`\`drast\n${insertText.replace(/\$\{\d+:([^}]+)\}/g, '$1')}\n\`\`\``
		}
	};
}

function builtin(name: string, completionKind: CompletionItemKind, signature: string, returnType: string, documentation: string): DrastSymbol {
	return {
		name,
		kind: 'builtin',
		scope: 'builtin',
		completionKind,
		uri: 'drast:builtin',
		range: Range.create(Position.create(0, 0), Position.create(0, name.length)),
		selectionRange: Range.create(Position.create(0, 0), Position.create(0, name.length)),
		definitionLine: signature,
		detail: `${returnType} ${name}`,
		signature,
		returnType,
		documentation
	};
}

function keywordCompletionItems(): CompletionItem[] {
	return Object.entries(keywordDescriptions).map(([keyword, description]) => ({
		label: keyword,
		kind: CompletionItemKind.Keyword,
		detail: 'Drast keyword',
		documentation: {
			kind: MarkupKind.Markdown,
			value: description
		}
	}));
}

function symbolCompletionItems(preferredUri: string): CompletionItem[] {
	const seen = new Set<string>();
	const items: CompletionItem[] = [];

	for (const symbol of allIndexedSymbols()) {
		if (symbol.scope === 'member' || symbol.scope === 'local') {
			continue;
		}

		const key = `${symbol.kind}:${symbol.qualifiedName ?? symbol.name}`;
		if (seen.has(key) || keywordDescriptions[symbol.name]) {
			continue;
		}

		seen.add(key);
		items.push(completionFromSymbol(symbol, preferredUri));
	}

	return items.sort(compareCompletionItems);
}

function localCompletionItems(document: TextDocument, position: Position): CompletionItem[] {
	const locals = visibleLocalSymbols(document, position);
	const seen = new Set<string>();
	const items: CompletionItem[] = [];

	for (const symbol of locals.reverse()) {
		if (seen.has(symbol.name)) {
			continue;
		}
		seen.add(symbol.name);
		items.push(completionFromSymbol(symbol, document.uri));
	}

	return items.sort(compareCompletionItems);
}

function baseCompletionItems(uri: string): CompletionItem[] {
	return [
		...snippetCompletions,
		...keywordCompletionItems(),
		...symbolCompletionItems(uri)
	];
}

function importCompletionItems(): CompletionItem[] {
	const headers = ['std', 'no_runtime', 'file native.hpp', 'iostream as std', 'vector', 'string', 'Arduino.h'];
	const workspaceModules = [...documentSymbols.keys()]
		.map(uriToFsPath)
		.filter((value): value is string => value !== undefined)
		.map(filePath => path.basename(filePath, '.drast'));

	return [...headers, ...workspaceModules].map((label) => ({
		label,
		kind: label.endsWith('.h') || label.includes('.hpp') ? CompletionItemKind.File : CompletionItemKind.Module,
		detail: 'Drast import',
		insertText: label
	}));
}

function memberCompletionItems(document: TextDocument, position: Position, context: CompletionContext): CompletionItem[] {
	if (context.shorthandVariant) {
		return enumVariantCompletionItems(undefined, true);
	}

	const receiver = context.receiver;
	if (!receiver) {
		return [];
	}

	const receiverType = resolveExpressionType(document, position, receiver);
	const items: CompletionItem[] = [];

	if (receiverType) {
		items.push(...membersForType(receiverType).map(symbol => completionFromSymbol(symbol, document.uri)));
		items.push(...enumVariantCompletionItems(receiverType, false));
		items.push(...builtinMembersForType(receiverType).map(symbol => completionFromSymbol(symbol, document.uri)));
	}

	items.push(...namespaceMembers(receiver).map(symbol => completionFromSymbol(symbol, document.uri)));

	if (items.length === 0) {
		items.push(...allMemberSymbols().map(symbol => completionFromSymbol(symbol, document.uri)));
	}

	return uniqueCompletionItems(items).sort(compareCompletionItems);
}

function completionFromSymbol(symbol: DrastSymbol, preferredUri: string): CompletionItem {
	const label = symbol.qualifiedName && symbol.kind === 'enumVariant' ? symbol.qualifiedName : symbol.name;
	return {
		label,
		kind: symbol.completionKind,
		detail: symbol.detail,
		documentation: {
			kind: MarkupKind.Markdown,
			value: symbolDocumentation(symbol, preferredUri)
		},
		insertText: symbol.insertText ?? symbol.name,
		sortText: symbol.sortText
	};
}

function symbolDocumentation(symbol: DrastSymbol, preferredUri: string): string {
	const language = symbol.kind.toString().startsWith('cpp') ? 'cpp' : 'drast';
	const source = symbol.uri === preferredUri ? 'Current file' : `Defined in \`${uriDisplayName(symbol.uri)}\``;
	const doc = symbol.documentation ? `\n\n${symbol.documentation}` : '';
	return `\`\`\`${language}\n${symbol.signature}\n\`\`\`\n\n${source}.${doc}`;
}

function hoverMarkdown(symbol: DrastSymbol): string {
	const language = symbol.kind.toString().startsWith('cpp') ? 'cpp' : 'drast';
	const location = symbol.uri.startsWith('drast:')
		? 'Drast runtime'
		: `${uriDisplayName(symbol.uri)}:${symbol.selectionRange.start.line + 1}`;
	const docs = symbol.documentation ? `\n\n${symbol.documentation}` : '';
	return `\`\`\`${language}\n${symbol.signature}\n\`\`\`\n\n**${symbolKindLabel(symbol)}** · ${location}${docs}`;
}

function symbolKindLabel(symbol: DrastSymbol): string {
	if (symbol.containerName) {
		return `${symbol.kind} of ${symbol.containerName}`;
	}

	return symbol.kind;
}

function compareCompletionItems(left: CompletionItem, right: CompletionItem): number {
	return String(left.label).localeCompare(String(right.label));
}

function uniqueCompletionItems(items: CompletionItem[]): CompletionItem[] {
	const seen = new Set<string>();
	const out: CompletionItem[] = [];
	for (const item of items) {
		const key = `${item.kind}:${String(item.label)}`;
		if (!seen.has(key)) {
			seen.add(key);
			out.push(item);
		}
	}
	return out;
}

interface SemanticTokenCandidate {
	line: number;
	start: number;
	length: number;
	type: SemanticTokenType;
}

function semanticTokensForDocument(uri: string, range?: Range): SemanticTokens {
	const document = documents.get(uri);
	if (!document) {
		return { data: [] };
	}

	const builder = new SemanticTokensBuilder();
	const candidates = collectSemanticTokenCandidates(document, range)
		.sort((left, right) => left.line - right.line || left.start - right.start || left.length - right.length);
	let previousLine = -1;
	let previousEnd = 0;

	for (const candidate of candidates) {
		const tokenType = semanticTokenTypeIndexes.get(candidate.type);
		if (tokenType === undefined || candidate.length <= 0) {
			continue;
		}

		if (candidate.line === previousLine && candidate.start < previousEnd) {
			continue;
		}

		builder.push(candidate.line, candidate.start, candidate.length, tokenType, 0);
		previousLine = candidate.line;
		previousEnd = candidate.start + candidate.length;
	}

	return builder.build();
}

function collectSemanticTokenCandidates(document: TextDocument, range?: Range): SemanticTokenCandidate[] {
	const text = document.getText();
	const lines = text.split(/\r?\n/);
	const candidates: SemanticTokenCandidate[] = [];
	const seen = new Set<string>();
	const add = (line: number, start: number, length: number, type: SemanticTokenType) => {
		if (line < 0 || line >= lines.length || start < 0 || !semanticTokenIntersectsRange(line, start, length, range)) {
			return;
		}

		const lineLength = lines[line]?.length ?? 0;
		if (length <= 0 || start + length > lineLength) {
			return;
		}

		const key = `${line}:${start}:${length}`;
		if (seen.has(key)) {
			return;
		}

		seen.add(key);
		candidates.push({ line, start, length, type });
	};

	for (const symbol of documentSymbols.get(document.uri) ?? []) {
		const tokenType = semanticTokenTypeForSymbol(symbol);
		if (!tokenType || symbol.uri !== document.uri) {
			continue;
		}

		add(
			symbol.selectionRange.start.line,
			symbol.selectionRange.start.character,
			symbol.selectionRange.end.character - symbol.selectionRange.start.character,
			tokenType
		);
	}

	collectSemanticReferences(document, lines, add);
	return candidates;
}

function semanticTokenIntersectsRange(line: number, start: number, length: number, range?: Range): boolean {
	if (!range) {
		return true;
	}

	const end = start + length;
	if (line < range.start.line || line > range.end.line) {
		return false;
	}

	if (line === range.start.line && end <= range.start.character) {
		return false;
	}

	if (line === range.end.line && start >= range.end.character) {
		return false;
	}

	return true;
}

function collectSemanticReferences(
	document: TextDocument,
	lines: string[],
	add: (line: number, start: number, length: number, type: SemanticTokenType) => void
): void {
	const commentState: CommentState = { blockDepth: 0 };

	for (let lineIndex = 0; lineIndex < lines.length; lineIndex += 1) {
		const codeLine = stripNonCode(lines[lineIndex], commentState);
		if (!codeLine.trim()) {
			continue;
		}

		addDottedSemanticReferences(document, lineIndex, codeLine, add);
		addStandaloneSemanticReferences(document, lineIndex, codeLine, add);
	}
}

function addDottedSemanticReferences(
	document: TextDocument,
	line: number,
	codeLine: string,
	add: (line: number, start: number, length: number, type: SemanticTokenType) => void
): void {
	const dottedRegex = /\b([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*)\.([A-Za-z_]\w*)\b/g;
	let match: RegExpExecArray | null;

	while ((match = dottedRegex.exec(codeLine)) !== null) {
		const receiver = match[1];
		const memberName = match[2];
		const memberStart = match.index + receiver.length + 1;
		const tokenType = semanticTokenTypeForMemberReference(document, Position.create(line, memberStart), receiver, memberName);
		if (tokenType) {
			add(line, memberStart, memberName.length, tokenType);
		}
	}

	const shorthandRegex = /(^|[^A-Za-z_0-9])\.([A-Za-z_]\w*)\b/g;
	while ((match = shorthandRegex.exec(codeLine)) !== null) {
		const variantName = match[2];
		if (findEnumVariant(variantName)) {
			add(line, match.index + match[1].length + 1, variantName.length, 'enumMember');
		}
	}
}

function semanticTokenTypeForMemberReference(
	document: TextDocument,
	position: Position,
	receiver: string,
	memberName: string
): SemanticTokenType | undefined {
	const receiverType = resolveExpressionType(document, position, receiver);
	if (receiverType) {
		const member = findMember(receiverType, memberName);
		if (member) {
			return semanticTokenTypeForMemberSymbol(member);
		}

		if (findEnumVariantForType(receiverType, memberName)) {
			return 'enumMember';
		}

		const builtinMember = builtinMembersForType(receiverType).find(symbol => symbol.name === memberName);
		if (builtinMember) {
			return semanticTokenTypeForMemberSymbol(builtinMember);
		}
	}

	const namespaceMember = namespaceMembers(receiver).find(symbol => terminalName(symbol.name) === memberName);
	if (namespaceMember) {
		return semanticTokenTypeForSymbol(namespaceMember);
	}

	return undefined;
}

function addStandaloneSemanticReferences(
	document: TextDocument,
	line: number,
	codeLine: string,
	add: (line: number, start: number, length: number, type: SemanticTokenType) => void
): void {
	const identifierRegex = /\b[A-Za-z_]\w*\b/g;
	let match: RegExpExecArray | null;

	while ((match = identifierRegex.exec(codeLine)) !== null) {
		const name = match[0];
		if (keywordDescriptions[name] || codeLine[match.index - 1] === '.') {
			continue;
		}

		const tokenType = semanticTokenTypeForName(document, Position.create(line, match.index), name);
		if (tokenType) {
			add(line, match.index, name.length, tokenType);
		}
	}
}

function semanticTokenTypeForName(document: TextDocument, position: Position, name: string): SemanticTokenType | undefined {
	const local = visibleLocalSymbols(document, position).reverse().find(symbol => symbol.name === name);
	if (local) {
		return semanticTokenTypeForSymbol(local);
	}

	const symbol = findSymbol(name, document.uri);
	if (symbol) {
		return semanticTokenTypeForSymbol(symbol);
	}

	if (primitiveTypes.includes(name) || looksLikeType(name)) {
		return 'type';
	}

	return undefined;
}

function semanticTokenTypeForSymbol(symbol: DrastSymbol): SemanticTokenType | undefined {
	if (symbol.kind === 'function' || symbol.kind === 'cppFunction') {
		return 'function';
	}

	if (symbol.kind === 'method') {
		return 'method';
	}

	if (symbol.kind === 'parameter') {
		return 'parameter';
	}

	if (symbol.kind === 'variable' || symbol.kind === 'cppVariable') {
		return 'variable';
	}

	if (symbol.kind === 'field') {
		return 'property';
	}

	if (symbol.kind === 'enumVariant') {
		return 'enumMember';
	}

	if (symbol.kind === 'struct' || symbol.kind === 'enum' || symbol.kind === 'protocol' || symbol.kind === 'cppType' || symbol.kind === 'cppEnum' || symbol.kind === 'typeParameter') {
		return 'type';
	}

	if (symbol.kind === 'namespace') {
		return 'namespace';
	}

	if (symbol.kind === 'builtin') {
		return symbol.completionKind === CompletionItemKind.Property ? 'property' : 'function';
	}

	return undefined;
}

function semanticTokenTypeForMemberSymbol(symbol: DrastSymbol): SemanticTokenType | undefined {
	if (symbol.kind === 'field' || symbol.completionKind === CompletionItemKind.Property) {
		return 'property';
	}

	if (symbol.kind === 'method' || symbol.completionKind === CompletionItemKind.Method || symbol.kind === 'builtin') {
		return 'method';
	}

	return semanticTokenTypeForSymbol(symbol);
}

function findEnumVariantForType(typeName: string, variantName: string): DrastSymbol | undefined {
	const variants = enumVariantTable.get(normalizeTypeName(typeName)) ?? enumVariantTable.get(typeName) ?? [];
	return variants.find(symbol => symbol.name === variantName || symbol.qualifiedName?.endsWith(`.${variantName}`));
}

function collectWorkspaceRoots(params: InitializeParams): string[] {
	const roots = new Set<string>();

	for (const folder of params.workspaceFolders ?? []) {
		roots.add(folder.uri);
	}

	if (params.rootUri) {
		roots.add(params.rootUri);
	}

	return [...roots];
}

async function ensureWorkspaceScanned(): Promise<void> {
	if (workspaceScanPromise) {
		await workspaceScanPromise;
		return;
	}

	if (!hasScannedWorkspace) {
		await scheduleWorkspaceScan();
	}
}

function scheduleWorkspaceScan(): Promise<void> {
	if (workspaceScanPromise) {
		return workspaceScanPromise;
	}

	workspaceScanPromise = scanWorkspace().finally(() => {
		workspaceScanPromise = undefined;
	});

	return workspaceScanPromise;
}

async function scanWorkspace(): Promise<void> {
	try {
		const seenUris = new Set<string>();

		for (const rootUri of workspaceRootUris) {
			const rootPath = uriToFsPath(rootUri);
			if (!rootPath) {
				continue;
			}

			for await (const filePath of walkDrastFiles(rootPath)) {
				const uri = pathToFileURL(filePath).toString();
				seenUris.add(uri);

				const openDocument = documents.get(uri);
				const text = openDocument ? openDocument.getText() : await fs.readFile(filePath, 'utf8');
				documentSymbols.set(uri, scanSymbolsInText(uri, text));
			}
		}

		for (const uri of [...documentSymbols.keys()]) {
			if (documents.get(uri)) {
				seenUris.add(uri);
				continue;
			}

			if (workspaceRootUris.length > 0 && !seenUris.has(uri)) {
				documentSymbols.delete(uri);
			}
		}

		rebuildIndexes();
		hasScannedWorkspace = true;
	} catch (error) {
		connection.console.error(`Drast workspace scan failed: ${String(error)}`);
	}
}

async function* walkDrastFiles(root: string): AsyncGenerator<string> {
	const ignored = new Set(['.git', 'node_modules', 'out', 'build', 'dist', '.xmake']);
	const entries = await fs.readdir(root, { withFileTypes: true });

	for (const entry of entries) {
		const fullPath = path.join(root, entry.name);
		if (entry.isDirectory()) {
			if (!ignored.has(entry.name)) {
				yield* walkDrastFiles(fullPath);
			}
			continue;
		}

		if (entry.isFile() && entry.name.endsWith('.drast')) {
			yield fullPath;
		}
	}
}

function updateDocumentSymbols(document: TextDocument): void {
	documentSymbols.set(document.uri, scanSymbolsInText(document.uri, document.getText()));
	rebuildIndexes();
	connection.sendDiagnostics({
		uri: document.uri,
		diagnostics: computeDiagnostics(document.getText())
	});
}

function rebuildIndexes(): void {
	symbolTable.clear();
	memberTable.clear();
	enumVariantTable.clear();

	for (const symbol of builtinSymbols) {
		addIndexedSymbol(symbol);
	}

	for (const symbols of documentSymbols.values()) {
		for (const symbol of symbols) {
			if (symbol.scope === 'local') {
				continue;
			}

			addIndexedSymbol(symbol);
		}
	}
}

function addIndexedSymbol(symbol: DrastSymbol): void {
	if (symbol.scope === 'member' && symbol.containerName) {
		const members = memberTable.get(symbol.containerName) ?? [];
		members.push(symbol);
		memberTable.set(symbol.containerName, members);
	}

	if (symbol.kind === 'enumVariant' && symbol.containerName) {
		const variants = enumVariantTable.get(symbol.containerName) ?? [];
		variants.push(symbol);
		enumVariantTable.set(symbol.containerName, variants);
	}

	for (const key of symbolKeys(symbol)) {
		const existing = symbolTable.get(key) ?? [];
		existing.push(symbol);
		symbolTable.set(key, existing);
	}
}

function symbolKeys(symbol: DrastSymbol): string[] {
	const keys = new Set<string>([symbol.name]);
	if (symbol.qualifiedName) {
		keys.add(symbol.qualifiedName);
	}
	return [...keys];
}

function allIndexedSymbols(): DrastSymbol[] {
	const seen = new Set<DrastSymbol>();
	const out: DrastSymbol[] = [];
	for (const symbols of symbolTable.values()) {
		for (const symbol of symbols) {
			if (!seen.has(symbol)) {
				seen.add(symbol);
				out.push(symbol);
			}
		}
	}
	return out;
}

function allMemberSymbols(): DrastSymbol[] {
	return [...memberTable.values()].flat();
}

function scanSymbolsInText(uri: string, text: string): DrastSymbol[] {
	const symbols: DrastSymbol[] = [];
	const lines = text.split(/\r?\n/);
	const contexts: BlockContext[] = [];
	const commentState: CommentState = { blockDepth: 0 };
	let pendingDoc: string[] = [];
	let docBlockActive = false;

	for (let lineIndex = 0; lineIndex < lines.length; lineIndex += 1) {
		const rawLine = lines[lineIndex];
		const rawTrimmed = rawLine.trim();
		const docLine = readDocCommentLine(rawTrimmed, docBlockActive);

		if (docLine) {
			pendingDoc.push(docLine.text);
			docBlockActive = docLine.continues;
			continue;
		}

		if (rawTrimmed.length === 0) {
			pendingDoc = [];
			continue;
		}

		const codeLine = stripNonCode(rawLine, commentState);
		const trimmed = codeLine.trim();
		if (trimmed.length === 0) {
			continue;
		}

		const indent = measureIndent(rawLine);
		while (contexts.length > 0 && indent <= contexts[contexts.length - 1].indent) {
			contexts.pop();
		}

		const currentContext = contexts[contexts.length - 1];
		const documentation = pendingDoc.length > 0 ? pendingDoc.join('\n') : undefined;
		pendingDoc = [];

		const useImport = parseUseImport(trimmed);
		if (useImport) {
			symbols.push(...symbolsForImport(uri, useImport, lineIndex));
			continue;
		}

		const typeDeclaration = parseTypeDeclaration(trimmed);
		if (typeDeclaration) {
			const character = codeLine.indexOf(typeDeclaration.name);
			symbols.push(makeSymbol({
				name: typeDeclaration.name,
				kind: typeDeclaration.kind,
				scope: 'global',
				completionKind: typeDeclaration.completionKind,
				uri,
				line: lineIndex,
				character,
				definitionLine: rawLine,
				detail: typeDeclaration.detail,
				signature: typeDeclaration.signature,
				documentation
			}));

			contexts.push({ indent, kind: typeDeclaration.contextKind, name: typeDeclaration.name, hostName: typeDeclaration.name });
			continue;
		}

		const implDeclaration = parseImplDeclaration(trimmed);
		if (implDeclaration) {
			contexts.push({ indent, kind: 'impl', name: implDeclaration.host, hostName: implDeclaration.host });
			continue;
		}

		if (trimmed === 'comptime' || trimmed.startsWith('comptime ')) {
			contexts.push({ indent, kind: 'comptime' });
			continue;
		}

		if (currentContext?.kind === 'enum' && indent > currentContext.indent && currentContext.name) {
			for (const variant of parseEnumVariants(trimmed, currentContext.name)) {
				symbols.push(makeSymbol({
					name: variant.name,
					qualifiedName: variant.qualifiedName,
					kind: 'enumVariant',
					scope: 'global',
					completionKind: CompletionItemKind.EnumMember,
					uri,
					line: lineIndex,
					character: Math.max(0, codeLine.indexOf(variant.rawName)),
					definitionLine: rawLine,
					detail: `${currentContext.name} variant`,
					signature: variant.signature,
					typeName: currentContext.name,
					containerName: currentContext.name,
					parameters: variant.parameters,
					documentation
				}));
			}
			continue;
		}

		if (currentContext?.kind === 'struct' && indent > currentContext.indent) {
			const field = parseFieldDeclaration(trimmed);
			if (field && currentContext.name) {
				symbols.push(makeSymbol({
					name: field.name,
					kind: 'field',
					scope: 'member',
					completionKind: CompletionItemKind.Field,
					uri,
					line: lineIndex,
					character: codeLine.indexOf(field.name),
					definitionLine: rawLine,
					detail: `${field.typeName} field`,
					signature: `${field.name}: ${field.typeName}`,
					typeName: field.typeName,
					containerName: currentContext.name,
					documentation
				}));
				continue;
			}
		}

		const shouldScanFunction =
			contexts.length === 0 ||
			(currentContext !== undefined &&
				(currentContext.kind === 'impl' || currentContext.kind === 'protocol') &&
				indent > currentContext.indent);

		if (shouldScanFunction) {
			const functionHeader = parseFunctionHeader(trimmed, currentContext?.hostName);
			if (functionHeader) {
				const kind: SymbolKind = currentContext?.kind === 'impl' || currentContext?.kind === 'protocol' ? 'method' : 'function';
				const scope: SymbolScope = kind === 'method' ? 'member' : 'global';
				const hostName = currentContext?.hostName;
				const symbolName = functionHeader.displayName;
				symbols.push(makeSymbol({
					name: symbolName,
					kind,
					scope,
					completionKind: kind === 'method' ? CompletionItemKind.Method : CompletionItemKind.Function,
					uri,
					line: lineIndex,
					character: codeLine.indexOf(functionHeader.name),
					definitionLine: rawLine,
					detail: functionDetail(functionHeader),
					signature: functionHeader.signature,
					returnType: functionHeader.returnType,
					parameters: functionHeader.parameters,
					containerName: hostName,
					documentation
				}));

				for (const parameter of functionHeader.parameters) {
					symbols.push(makeSymbol({
						name: parameter.name,
						kind: 'parameter',
						scope: 'local',
						completionKind: CompletionItemKind.Variable,
						uri,
						line: lineIndex,
						character: Math.max(0, codeLine.indexOf(parameter.name)),
						definitionLine: rawLine,
						detail: `${parameter.typeName ?? 'auto'} parameter`,
						signature: `${parameter.name}: ${parameter.typeName ?? 'auto'}`,
						typeName: parameter.typeName,
						containerName: functionHeader.displayName
					}));
				}

				contexts.push({ indent, kind: 'function', name: functionHeader.displayName, hostName });
				continue;
			}
		}

		if (contexts.some(context => context.kind === 'function')) {
			const variable = parseVariableDeclaration(trimmed, codeLine);
			if (variable) {
				symbols.push(makeSymbol({
					name: variable.name,
					kind: 'variable',
					scope: 'local',
					completionKind: CompletionItemKind.Variable,
					uri,
					line: lineIndex,
					character: variable.character,
					definitionLine: rawLine,
					detail: variable.detail,
					signature: variable.signature,
					typeName: variable.typeName
				}));
			}
		} else if (contexts.length === 0) {
			const variable = parseVariableDeclaration(trimmed, codeLine);
			if (variable) {
				symbols.push(makeSymbol({
					name: variable.name,
					kind: 'variable',
					scope: 'global',
					completionKind: CompletionItemKind.Variable,
					uri,
					line: lineIndex,
					character: variable.character,
					definitionLine: rawLine,
					detail: variable.detail,
					signature: variable.signature,
					typeName: variable.typeName,
					documentation
				}));
			}
		}
	}

	return symbols;
}

function parseTypeDeclaration(trimmedLine: string):
	| {
		name: string;
		kind: SymbolKind;
		completionKind: CompletionItemKind;
		detail: string;
		signature: string;
		contextKind: BlockContext['kind'];
	}
	| undefined {
	const match = trimmedLine.match(/^(?:fileprivate\s+)?(struct|enum|protocol)\s+([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)?)(`(?:\[[^\]]*\]))?/);
	if (!match) {
		return undefined;
	}

	const declarationKind = match[1];
	const name = match[2];
	const generics = match[3] ?? '';

	if (declarationKind === 'struct') {
		return {
			name,
			kind: 'struct',
			completionKind: CompletionItemKind.Class,
			detail: 'Drast struct',
			signature: `struct ${name}${generics}`,
			contextKind: 'struct'
		};
	}

	if (declarationKind === 'enum') {
		return {
			name,
			kind: 'enum',
			completionKind: CompletionItemKind.Enum,
			detail: 'Drast enum',
			signature: `enum ${name}${generics}`,
			contextKind: 'enum'
		};
	}

	return {
		name,
		kind: 'protocol',
		completionKind: CompletionItemKind.Interface,
		detail: 'Drast protocol',
		signature: `protocol ${name}`,
		contextKind: 'protocol'
	};
}

function parseImplDeclaration(trimmedLine: string): { host: string; protocol?: string } | undefined {
	const match = trimmedLine.match(/^impl\s+([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)?)(?:\s+as\s+([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)?))?/);
	if (!match) {
		return undefined;
	}

	return { host: match[1], protocol: match[2] };
}

function parseFunctionHeader(trimmedLine: string, hostName?: string): FunctionHeader | undefined {
	if (assignmentOperatorIndex(trimmedLine) >= 0 || trimmedLine.startsWith('use ')) {
		return undefined;
	}

	if (/;\s*$/.test(trimmedLine) && !trimmedLine.includes('operator[') && !trimmedLine.includes(',')) {
		return undefined;
	}

	let line = trimmedLine;
	let visibility: string | undefined;
	const visibilityMatch = line.match(/^(private|preview)\s+/);
	if (visibilityMatch) {
		visibility = visibilityMatch[1];
		line = line.slice(visibilityMatch[0].length).trimStart();
	}

	const operatorMatch = line.match(/^operator\[([^\]]+)\](.*)$/);
	if (operatorMatch) {
		const operatorSymbol = operatorMatch[1].trim();
		const tail = operatorMatch[2].trim();
		const parsed = parseFunctionTail(`operator${operatorSymbol}`, tail, hostName);
		return {
			...parsed,
			name: 'operator',
			displayName: `operator${operatorSymbol}`,
			visibility,
			isOperator: true,
			operatorSymbol,
			signature: `${visibility ? `${visibility} ` : ''}operator[${operatorSymbol}](${formatParameters(parsed.parameters)}) -> ${parsed.returnType}`
		};
	}

	const match = line.match(/^([A-Za-z_]\w*)(`(?:\[[^\]]*\]))?(.*)$/);
	if (!match) {
		return undefined;
	}

	const name = match[1];
	if (declarationKeywords.has(name)) {
		return undefined;
	}

	const genericText = match[2] ?? '';
	const typeParameters = genericText ? genericText.slice(2, -1).split(/[\s,]+/).filter(Boolean) : [];
	const tail = match[3].trim();
	const parsed = parseFunctionTail(name, tail, hostName);
	const returnType = name === 'init' && hostName ? hostName : parsed.returnType;
	const displayName = name;
	const genericDisplay = typeParameters.length > 0 ? `\`[${typeParameters.join(' ')}]` : '';

	return {
		...parsed,
		name,
		displayName,
		typeParameters,
		returnType,
		visibility,
		isOperator: false,
		signature: `${visibility ? `${visibility} ` : ''}${name}${genericDisplay}(${formatParameters(parsed.parameters)}) -> ${returnType}${parsed.discard ? ' discard' : ''}`
	};
}

function parseFunctionTail(name: string, tail: string, hostName?: string): Omit<FunctionHeader, 'name' | 'displayName' | 'visibility' | 'isOperator' | 'signature'> {
	const parts = splitOutsideDelimiters(tail, ',');
	const parameterText = parts[0]?.trim() ?? '';
	const returnText = parts.length > 1 ? parts.slice(1).join(',').trim() : '';
	const returnTokens = returnText.split(/\s+/).filter(Boolean);
	const discard = returnTokens.includes('discard');
	const cleanedReturn = returnTokens.filter(token => token !== 'discard').join(' ');
	const parameters = parseParameters(parameterText);
	const returnType = cleanedReturn || (name === 'init' && hostName ? hostName : 'void');

	return {
		typeParameters: [],
		parameters,
		returnType,
		discard,
		operatorSymbol: undefined
	};
}

function parseParameters(text: string): DrastParameter[] {
	if (!text.trim()) {
		return [];
	}

	return splitWhitespaceOutsideDelimiters(text)
		.map(token => parseParameter(token))
		.filter((parameter): parameter is DrastParameter => parameter !== undefined);
}

function parseParameter(token: string): DrastParameter | undefined {
	const pieces = token.split(';');
	const name = pieces[0]?.trim();
	if (!name || !/^[A-Za-z_]\w*$/.test(name)) {
		return undefined;
	}

	return {
		name,
		typeName: pieces[1]?.trim() || undefined,
		defaultValue: pieces[2]?.trim() || undefined
	};
}

function formatParameters(parameters: DrastParameter[]): string {
	return parameters.map(parameter => {
		const typeText = parameter.typeName ? `: ${parameter.typeName}` : ': auto';
		const defaultText = parameter.defaultValue ? ` = ${parameter.defaultValue}` : '';
		return `${parameter.name}${typeText}${defaultText}`;
	}).join(', ');
}

function functionDetail(header: FunctionHeader): string {
	return `${header.returnType} ${header.displayName}(${formatParameters(header.parameters)})`;
}

function parseFieldDeclaration(trimmedLine: string): { name: string; typeName: string; visibility?: string } | undefined {
	if (trimmedLine.includes('=') || trimmedLine.startsWith('enum ') || trimmedLine.startsWith('struct ')) {
		return undefined;
	}

	const match = trimmedLine.replace(/;$/, '').match(/^(?:(private|preview)\s+)?([A-Za-z_]\w*)\s+(.+)$/);
	if (!match) {
		return undefined;
	}

	const typeName = match[3].trim();
	if (!looksLikeType(typeName)) {
		return undefined;
	}

	return {
		visibility: match[1],
		name: match[2],
		typeName
	};
}

function parseEnumVariants(trimmedLine: string, enumName: string): Array<{ name: string; rawName: string; qualifiedName: string; signature: string; parameters?: DrastParameter[] }> {
	const semicolonVariants = trimmedLine.split(';').map(part => part.trim()).filter(Boolean);
	if (semicolonVariants.length > 1 || (semicolonVariants.length === 1 && trimmedLine.endsWith(';') && !semicolonVariants[0].includes(' '))) {
		return semicolonVariants.map(rawName => enumVariant(rawName, enumName));
	}

	const tokens = splitWhitespaceOutsideDelimiters(trimmedLine.replace(/;$/, ''));
	const firstParameterIndex = tokens.findIndex(token => token.includes(';'));
	if (firstParameterIndex > 0) {
		const rawName = tokens.slice(0, firstParameterIndex).join(' ');
		const parameters = tokens.slice(firstParameterIndex)
			.map(token => parseParameter(token))
			.filter((parameter): parameter is DrastParameter => parameter !== undefined);
		return [enumVariant(rawName, enumName, parameters)];
	}

	return [enumVariant(trimmedLine.replace(/;$/, '').trim(), enumName)];
}

function enumVariant(rawName: string, enumName: string, parameters?: DrastParameter[]): { name: string; rawName: string; qualifiedName: string; signature: string; parameters?: DrastParameter[] } {
	const name = sanitizeEnumVariantName(rawName);
	const payload = parameters && parameters.length > 0 ? `(${formatParameters(parameters)})` : '';
	return {
		name,
		rawName,
		qualifiedName: `${enumName}.${name}`,
		signature: `${enumName}.${name}${payload}`,
		parameters
	};
}

function sanitizeEnumVariantName(name: string): string {
	return name.trim().replace(/\s+/g, '_').replace(/[^\w]/g, '_').replace(/^_+|_+$/g, '');
}

function parseVariableDeclaration(trimmedLine: string, codeLine: string): VariableDeclaration | undefined {
	if (!trimmedLine || declarationKeywords.has(trimmedLine.split(/\s+/)[0] ?? '')) {
		return undefined;
	}

	if (trimmedLine.includes('.') && assignmentOperatorIndex(trimmedLine) >= 0 && trimmedLine.indexOf('.') < assignmentOperatorIndex(trimmedLine)) {
		return undefined;
	}

	const uninitialized = trimmedLine.match(/^([A-Za-z_]\w*)\s+(.+);$/);
	if (uninitialized && looksLikeType(uninitialized[2].trim())) {
		const typeName = uninitialized[2].trim();
		return {
			name: uninitialized[1],
			character: codeLine.indexOf(uninitialized[1]),
			detail: `${typeName} variable`,
			typeName,
			signature: `${uninitialized[1]}: ${typeName}`
		};
	}

	const equalIndex = assignmentOperatorIndex(trimmedLine);
	if (equalIndex >= 0) {
		const beforeEqual = trimmedLine.slice(0, equalIndex).trim();
		if (!beforeEqual) {
			return undefined;
		}

		const typed = beforeEqual.match(/^([A-Za-z_]\w*)\s+(.+)$/);
		const inferred = beforeEqual.match(/^([A-Za-z_]\w*)(?:\s+const)?$/);
		if (typed && looksLikeType(typed[2].trim())) {
			const typeName = typed[2].replace(/\bconst\b/g, '').trim();
			return {
				name: typed[1],
				character: codeLine.indexOf(typed[1]),
				detail: `${typeName} variable`,
				typeName,
				signature: `${typed[1]}: ${typeName}`
			};
		}

		if (beforeEqual.includes('.') || beforeEqual.includes('[')) {
			return undefined;
		}

		if (inferred) {
			const initializer = trimmedLine.slice(equalIndex + 1).trim();
			const typeName = inferTypeFromInitializer(initializer);
			return {
				name: inferred[1],
				character: codeLine.indexOf(inferred[1]),
				detail: `${typeName ?? 'auto'} variable`,
				typeName,
				signature: `${inferred[1]}: ${typeName ?? 'auto'}`
			};
		}
	}

	return undefined;
}

function looksLikeType(text: string): boolean {
	const type = text.trim();
	if (!type) {
		return false;
	}

	return primitiveTypes.includes(type) ||
		type.startsWith('{') ||
		type.startsWith('~') ||
		type.startsWith('@[') ||
		type.startsWith('maybe ') ||
		type.startsWith('variadic[') ||
		type.startsWith('tuple ') ||
		type.startsWith('map`[') ||
		type.includes('::') ||
		type.includes('`[') ||
		type.endsWith('`') ||
		/^[A-Z_][A-Za-z_0-9]*(?:\.[A-Z_][A-Za-z_0-9]*)?$/.test(type);
}

function inferTypeFromInitializer(initializer: string): string | undefined {
	const arrayLiteralType = inferArrayLiteralType(initializer);
	if (arrayLiteralType) {
		return arrayLiteralType;
	}

	if (/^s?'(?:\\.|[^'\\])*'$/.test(initializer)) {
		const content = initializer.replace(/^s?'/, '').replace(/'$/, '');
		return initializer.startsWith("s'") || [...content].length !== 1 ? 'string' : 'char';
	}

	if (/^-?\d+\.\d+$/.test(initializer)) {
		return 'double';
	}

	if (/^-?\d+$/.test(initializer)) {
		return 'int';
	}

	if (initializer === 'true' || initializer === 'false') {
		return 'bool';
	}

	if (initializer === 'nil') {
		return 'maybe';
	}

	const heap = initializer.match(/^@\[([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)?)/);
	if (heap) {
		return heap[1];
	}

	const constructor = initializer.match(/^([A-Z_][A-Za-z_0-9]*(?:\.[A-Z_][A-Za-z_0-9]*)?)\[/);
	if (constructor) {
		return constructor[1];
	}

	const variant = initializer.match(/^([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)?)\.[A-Za-z_]\w*/);
	if (variant) {
		return variant[1];
	}

	const call = initializer.match(/^([A-Za-z_]\w*)\b/);
	if (call) {
		return findSymbol(call[1], '')?.returnType;
	}

	return undefined;
}

function inferArrayLiteralType(initializer: string): string | undefined {
	const trimmed = initializer.trim();
	if (!trimmed.startsWith('{') || !trimmed.endsWith('}')) {
		return undefined;
	}

	const items = splitTopLevelItems(trimmed.slice(1, -1));
	if (items.length === 0) {
		return '{auto}';
	}

	if (items.length === 1 && looksLikeType(items[0])) {
		return `{${items[0]}}`;
	}

	const itemTypes = items.map(item => inferTypeFromInitializer(item));
	const firstType = itemTypes[0];
	if (firstType && itemTypes.every(typeName => typeName === firstType)) {
		return `{${firstType}}`;
	}

	return '{auto}';
}

function parseUseImport(trimmedLine: string): UseImport | undefined {
	const match = trimmedLine.match(/^use(?:\s+(file))?\s+(.+)$/);
	if (!match) {
		return undefined;
	}

	let rest = match[2].trim();
	let alias: string | undefined;
	const aliasMatch = rest.match(/^(.+?)\s+as\s+([A-Za-z_]\w*)$/);
	if (aliasMatch) {
		rest = aliasMatch[1].trim();
		alias = aliasMatch[2];
	}

	const quoted = rest.match(/^['"](.+)['"]$/);
	const pathText = quoted ? quoted[1] : rest;
	const isHeader = /\.(?:h|hpp|hh|hxx)$/.test(pathText) || ['iostream', 'vector', 'string', 'map', 'unordered_map'].includes(pathText);

	return {
		raw: trimmedLine,
		pathText,
		isHeader,
		isQuoted: Boolean(quoted),
		isFile: Boolean(match[1]),
		alias
	};
}

function symbolsForImport(uri: string, useImport: UseImport, line: number): DrastSymbol[] {
	if (!useImport.isHeader) {
		return [];
	}

	const filePath = resolveHeaderPath(uri, useImport);
	if (filePath && syncFs.existsSync(filePath)) {
		return parseHeaderSymbols(filePath, useImport.alias);
	}

	return systemHeaderSymbols(useImport.pathText, useImport.alias, uri, line);
}

function resolveHeaderPath(uri: string, useImport: UseImport): string | undefined {
	const currentFile = uriToFsPath(uri);
	const currentDir = currentFile ? path.dirname(currentFile) : undefined;
	const candidates: string[] = [];

	if (path.isAbsolute(useImport.pathText)) {
		candidates.push(useImport.pathText);
	} else {
		if (currentDir) {
			candidates.push(path.join(currentDir, useImport.pathText));
		}

		for (const rootUri of workspaceRootUris) {
			const rootPath = uriToFsPath(rootUri);
			if (rootPath) {
				candidates.push(path.join(rootPath, useImport.pathText));
			}
		}
	}

	return candidates.find(candidate => syncFs.existsSync(candidate));
}

function parseHeaderSymbols(filePath: string, alias?: string): DrastSymbol[] {
	const cacheKey = `${filePath}:${alias ?? ''}`;
	const cached = headerSymbolCache.get(cacheKey);
	if (cached) {
		return cached;
	}

	let text = '';
	try {
		text = syncFs.readFileSync(filePath, 'utf8');
	} catch {
		return [];
	}

	const uri = pathToFileURL(filePath).toString();
	const symbols: DrastSymbol[] = [];
	const lines = text.split(/\r?\n/);
	let pendingDoc: string[] = [];
	let docBlockActive = false;

	for (let lineIndex = 0; lineIndex < lines.length; lineIndex += 1) {
		const rawLine = lines[lineIndex];
		const trimmed = rawLine.trim();
		const docLine = readDocCommentLine(trimmed, docBlockActive);
		if (docLine) {
			pendingDoc.push(docLine.text);
			docBlockActive = docLine.continues;
			continue;
		}

		if (!trimmed || trimmed.startsWith('#')) {
			if (!trimmed) {
				pendingDoc = [];
			}
			continue;
		}

		const documentation = pendingDoc.length > 0 ? pendingDoc.join('\n') : undefined;
		pendingDoc = [];
		const line = stripCppLineComment(trimmed);

		const typeMatch = line.match(/^(?:template\s*<[^>]+>\s*)?(class|struct|enum(?:\s+class)?)\s+([A-Za-z_]\w*)/);
		if (typeMatch) {
			const name = aliasQualified(alias, typeMatch[2]);
			symbols.push(makeSymbol({
				name,
				kind: typeMatch[1].startsWith('enum') ? 'cppEnum' : 'cppType',
				scope: 'import',
				completionKind: typeMatch[1].startsWith('enum') ? CompletionItemKind.Enum : CompletionItemKind.Class,
				uri,
				line: lineIndex,
				character: rawLine.indexOf(typeMatch[2]),
				definitionLine: rawLine,
				detail: `C++ ${typeMatch[1]}`,
				signature: line.replace(/\s*\{.*$/, ''),
				documentation
			}));
			continue;
		}

		const functionMatch = line.match(/^(?:template\s*<[^>]+>\s*)?(?:(?:inline|static|extern|constexpr|virtual|friend)\s+)*([~\w:<>,&*\s]+?)\s+([A-Za-z_]\w*(?:::[A-Za-z_]\w*)?)\s*\(([^)]*)\)\s*(?:const)?\s*(?:noexcept)?\s*(?:[;{=]|$)/);
		if (functionMatch && !['if', 'while', 'for', 'switch'].includes(functionMatch[2])) {
			const name = aliasQualified(alias, functionMatch[2].split('::').pop() ?? functionMatch[2]);
			const returnType = functionMatch[1].trim();
			const params = functionMatch[3].trim();
			symbols.push(makeSymbol({
				name,
				qualifiedName: alias ? `${alias}.${name}` : undefined,
				kind: 'cppFunction',
				scope: 'import',
				completionKind: CompletionItemKind.Function,
				uri,
				line: lineIndex,
				character: rawLine.indexOf(functionMatch[2]),
				definitionLine: rawLine,
				detail: `${returnType} ${name}(${params})`,
				signature: `${returnType} ${name}(${params})`,
				returnType,
				documentation
			}));
			continue;
		}

		const variableMatch = line.match(/^(?:(?:extern|static|inline|constexpr)\s+)*(const\s+)?([A-Za-z_][\w:<>,&*\s]*?)\s+([A-Za-z_]\w*)\s*(?:=|;)/);
		if (variableMatch) {
			const name = aliasQualified(alias, variableMatch[3]);
			const typeName = `${variableMatch[1] ?? ''}${variableMatch[2]}`.trim();
			symbols.push(makeSymbol({
				name,
				kind: 'cppVariable',
				scope: 'import',
				completionKind: CompletionItemKind.Variable,
				uri,
				line: lineIndex,
				character: rawLine.indexOf(variableMatch[3]),
				definitionLine: rawLine,
				detail: `${typeName} ${name}`,
				signature: `${typeName} ${name}`,
				typeName,
				documentation
			}));
		}
	}

	headerSymbolCache.set(cacheKey, symbols);
	return symbols;
}

function aliasQualified(alias: string | undefined, name: string): string {
	return alias ? `${alias}.${name}` : name;
}

function systemHeaderSymbols(header: string, alias: string | undefined, uri: string, line: number): DrastSymbol[] {
	const namespaceName = alias ?? (header === 'iostream' ? 'std' : undefined);
	const symbols: DrastSymbol[] = [];

	if (namespaceName) {
		symbols.push(makeSymbol({
			name: namespaceName,
			kind: 'namespace',
			scope: 'import',
			completionKind: CompletionItemKind.Module,
			uri,
			line,
			character: 0,
			definitionLine: `use ${header} as ${namespaceName}`,
			detail: `C++ namespace alias for ${header}`,
			signature: `namespace ${namespaceName} = ${header}`,
			documentation: `Symbols from the C++ \`${header}\` header exposed through Drast.`
		}));
	}

	if (header === 'Arduino.h') {
		return [
			...symbols,
			cppBuiltin(uri, line, 'pinMode', 'void pinMode(int pin, int mode)', 'Configure an Arduino pin mode.'),
			cppBuiltin(uri, line, 'digitalWrite', 'void digitalWrite(int pin, int value)', 'Write HIGH or LOW to a digital pin.'),
			cppBuiltin(uri, line, 'delay', 'void delay(unsigned long ms)', 'Pause execution for the given milliseconds.'),
			cppBuiltin(uri, line, 'OUTPUT', 'int OUTPUT', 'Arduino output-pin mode constant.'),
			cppBuiltin(uri, line, 'HIGH', 'int HIGH', 'Arduino high digital value.'),
			cppBuiltin(uri, line, 'LOW', 'int LOW', 'Arduino low digital value.'),
			cppBuiltin(uri, line, 'Serial', 'HardwareSerial Serial', 'Arduino serial port object.')
		];
	}

	if (header === 'iostream' || namespaceName === 'std') {
		return [
			...symbols,
			cppBuiltin(uri, line, `${namespaceName ?? 'std'}.cout`, 'std::ostream cout', 'Standard output stream.'),
			cppBuiltin(uri, line, `${namespaceName ?? 'std'}.cin`, 'std::istream cin', 'Standard input stream.'),
			cppBuiltin(uri, line, `${namespaceName ?? 'std'}.cerr`, 'std::ostream cerr', 'Standard error stream.'),
			cppBuiltin(uri, line, `${namespaceName ?? 'std'}.endl`, 'std::ostream& endl(std::ostream&)', 'Stream newline manipulator.')
		];
	}

	return symbols;
}

function cppBuiltin(uri: string, line: number, name: string, signature: string, documentation: string): DrastSymbol {
	return makeSymbol({
		name,
		qualifiedName: name,
		kind: signature.includes('(') ? 'cppFunction' : 'cppVariable',
		scope: 'import',
		completionKind: signature.includes('(') ? CompletionItemKind.Function : CompletionItemKind.Variable,
		uri,
		line,
		character: 0,
		definitionLine: signature,
		detail: signature,
		signature,
		documentation
	});
}

function makeSymbol(input: {
	name: string;
	qualifiedName?: string;
	kind: SymbolKind;
	scope: SymbolScope;
	completionKind: CompletionItemKind;
	uri: string;
	line: number;
	character: number;
	definitionLine: string;
	detail: string;
	signature: string;
	returnType?: string;
	typeName?: string;
	parameters?: DrastParameter[];
	containerName?: string;
	documentation?: string;
	insertText?: string;
	sortText?: string;
}): DrastSymbol {
	const safeCharacter = Math.max(0, input.character);
	const range = Range.create(
		Position.create(input.line, 0),
		Position.create(input.line, input.definitionLine.length)
	);
	const selectionRange = Range.create(
		Position.create(input.line, safeCharacter),
		Position.create(input.line, safeCharacter + input.name.length)
	);

	return {
		name: input.name,
		qualifiedName: input.qualifiedName,
		kind: input.kind,
		scope: input.scope,
		completionKind: input.completionKind,
		uri: input.uri,
		range,
		selectionRange,
		definitionLine: input.definitionLine,
		detail: input.detail,
		signature: input.signature,
		returnType: input.returnType,
		typeName: input.typeName,
		parameters: input.parameters,
		containerName: input.containerName,
		documentation: input.documentation,
		insertText: input.insertText,
		sortText: input.sortText
	};
}

function resolveSymbolAtToken(document: TextDocument, position: Position, text: string): DrastSymbol | undefined {
	const memberAccess = memberAccessAtPosition(document, position);
	if (memberAccess) {
		const receiverType = resolveExpressionType(document, position, memberAccess.receiver);
		if (receiverType) {
			const member = findMember(receiverType, memberAccess.member);
			if (member) {
				return member;
			}
		}

		const namespaceMember = namespaceMembers(memberAccess.receiver).find(symbol => terminalName(symbol.name) === memberAccess.member);
		if (namespaceMember) {
			return namespaceMember;
		}
	}

	const local = visibleLocalSymbols(document, position).reverse().find(symbol => symbol.name === text);
	if (local) {
		return local;
	}

	if (text.startsWith('.')) {
		return findEnumVariant(text.slice(1));
	}

	return findSymbol(text, document.uri);
}

function findSymbol(name: string, preferredUri: string): DrastSymbol | undefined {
	const symbols = symbolTable.get(name);
	if (!symbols || symbols.length === 0) {
		return undefined;
	}

	return symbols.find(symbol => symbol.uri === preferredUri) ??
		symbols.find(symbol => symbol.scope !== 'builtin') ??
		symbols[0];
}

function findMember(typeName: string, memberName: string): DrastSymbol | undefined {
	const normalized = normalizeTypeName(typeName);
	const members = membersForType(normalized);
	return members.find(symbol => symbol.name === memberName || terminalName(symbol.name) === memberName);
}

function findEnumVariant(variantName: string): DrastSymbol | undefined {
	for (const variants of enumVariantTable.values()) {
		const found = variants.find(symbol => symbol.name === variantName || symbol.qualifiedName?.endsWith(`.${variantName}`));
		if (found) {
			return found;
		}
	}
	return undefined;
}

function membersForType(typeName: string): DrastSymbol[] {
	const normalized = normalizeTypeName(typeName);
	return memberTable.get(normalized) ?? memberTable.get(typeName) ?? [];
}

function enumVariantCompletionItems(typeName: string | undefined, shorthand: boolean): CompletionItem[] {
	const variants = typeName ? enumVariantTable.get(normalizeTypeName(typeName)) ?? [] : [...enumVariantTable.values()].flat();
	return variants.map(symbol => {
		const label = shorthand ? `.${symbol.name}` : symbol.name;
		return {
			...completionFromSymbol(symbol, symbol.uri),
			label,
			insertText: symbol.name,
			detail: symbol.detail
		};
	});
}

function builtinMembersForType(typeName: string): DrastSymbol[] {
	const normalized = normalizeTypeName(typeName);
	if (['string', 'String', 'std::string'].includes(normalized)) {
		return [
			memberBuiltin('length', 'usize', 'string.length', 'Number of characters.'),
			memberBuiltin('lineCount', 'usize', 'string.lineCount', 'Number of lines.'),
			memberBuiltin('splitWhitespace', '{string}', 'string.splitWhitespace', 'Split on whitespace.'),
			memberBuiltin('trim', 'string', 'string.trim', 'Trim leading and trailing whitespace.'),
			memberBuiltin('asInt', 'maybe int', 'string.asInt', 'Parse as an integer.'),
			memberBuiltin('lowercase', 'string', 'string.lowercase', 'Lowercase copy.'),
			memberBuiltin('contains', 'bool', 'string.contains needle;string', 'Substring membership test.'),
			memberBuiltin('startsWith', 'bool', 'string.startsWith prefix;string', 'Prefix test.'),
			memberBuiltin('endsWith', 'bool', 'string.endsWith suffix;string', 'Suffix test.'),
			memberBuiltin('find', 'int', 'string.find needle;string', 'Find a substring.'),
			memberBuiltin('replace', 'string', 'string.replace needle;string replacement;string', 'Replace substrings.'),
			memberBuiltin('split', '{string}', 'string.split delimiter;string', 'Split by delimiter.'),
			memberBuiltin('substring', 'string', 'string.substring from;int to;int', 'Extract a substring.')
		];
	}

	if (isVectorLikeType(normalized)) {
		return [
			memberBuiltin('length', 'usize', 'array.length', 'Number of elements.'),
			memberBuiltin('contains', 'bool', 'array.contains value;T', 'Element membership test.'),
			memberBuiltin('removeAt', 'void', 'array.removeAt index;usize', 'Remove an element by index.'),
			memberBuiltin('remove', 'void', 'array.remove value;T', 'Remove matching elements.')
		];
	}

	if (normalized.startsWith('map`[') || normalized.startsWith('std::unordered_map')) {
		return [
			memberBuiltin('keys', '{K}', 'map.keys', 'Return map keys.'),
			memberBuiltin('values', '{V}', 'map.values', 'Return map values.'),
			memberBuiltin('contains', 'bool', 'map.contains key;K', 'Return whether a key exists.'),
			memberBuiltin('get', 'V', 'map.get key;K fallback;V', 'Return a value or fallback.'),
			memberBuiltin('set', 'void', 'map.set key;K value;V', 'Set a map value.'),
			memberBuiltin('clear', 'void', 'map.clear', 'Remove all entries.')
		];
	}

	if (normalized.startsWith('maybe ') || normalized.startsWith('std::optional')) {
		return [
			memberBuiltin('value', 'T', 'optional.value', 'Force unwrap the optional.'),
			memberBuiltin('valueOr', 'T', 'optional.valueOr fallback;T', 'Return contained value or fallback.')
		];
	}

	return [];
}

function isVectorLikeType(typeName: string): boolean {
	const normalized = normalizeTypeName(typeName);
	return normalized.startsWith('{') ||
		normalized.startsWith('std::vector') ||
		normalized.startsWith('vector`[') ||
		normalized.startsWith('Vector`[');
}

function memberBuiltin(name: string, returnType: string, signature: string, documentation: string): DrastSymbol {
	return {
		name,
		kind: 'builtin',
		scope: 'builtin',
		completionKind: signature.includes(' ') ? CompletionItemKind.Method : CompletionItemKind.Property,
		uri: 'drast:builtin-member',
		range: Range.create(Position.create(0, 0), Position.create(0, name.length)),
		selectionRange: Range.create(Position.create(0, 0), Position.create(0, name.length)),
		definitionLine: signature,
		detail: returnType,
		signature: `${signature} -> ${returnType}`,
		returnType,
		documentation
	};
}

function namespaceMembers(receiver: string): DrastSymbol[] {
	const prefix = `${receiver}.`;
	return allIndexedSymbols().filter(symbol => symbol.name.startsWith(prefix) || symbol.qualifiedName?.startsWith(prefix));
}

function terminalName(name: string): string {
	return name.split('.').pop() ?? name;
}

function normalizeTypeName(typeName: string): string {
	return typeName
		.replace(/^maybe\s+/, '')
		.replace(/^@\[([^\]\s]+).*$/, '$1')
		.replace(/^~+/, '')
		.replace(/`+$/, '')
		.trim();
}

function visibleLocalSymbols(document: TextDocument, position: Position): DrastSymbol[] {
	const symbols = documentSymbols.get(document.uri) ?? [];
	const functionStart = currentFunctionStartLine(document, position);
	return symbols.filter(symbol =>
		symbol.scope === 'local' &&
		(functionStart === undefined || symbol.selectionRange.start.line >= functionStart) &&
		symbol.selectionRange.start.line <= position.line
	);
}

function currentFunctionStartLine(document: TextDocument, position: Position): number | undefined {
	const lines = document.getText(Range.create(Position.create(0, 0), Position.create(position.line + 1, 0))).split(/\r?\n/);
	const contexts: BlockContext[] = [];
	const commentState: CommentState = { blockDepth: 0 };

	for (let lineIndex = 0; lineIndex < lines.length; lineIndex += 1) {
		const rawLine = lines[lineIndex];
		const codeLine = stripNonCode(rawLine, commentState);
		const trimmed = codeLine.trim();
		if (!trimmed) {
			continue;
		}

		const indent = measureIndent(rawLine);
		while (contexts.length > 0 && indent <= contexts[contexts.length - 1].indent) {
			contexts.pop();
		}

		const current = contexts[contexts.length - 1];
		const type = parseTypeDeclaration(trimmed);
		if (type) {
			contexts.push({ indent, kind: type.contextKind, name: type.name, hostName: type.name, line: lineIndex });
			continue;
		}

		const impl = parseImplDeclaration(trimmed);
		if (impl) {
			contexts.push({ indent, kind: 'impl', hostName: impl.host, name: impl.host, line: lineIndex });
			continue;
		}

		const shouldScanFunction =
			contexts.length === 0 ||
			(current !== undefined &&
				(current.kind === 'impl' || current.kind === 'protocol') &&
				indent > current.indent);
		if (shouldScanFunction) {
			const header = parseFunctionHeader(trimmed, current?.hostName);
			if (header) {
				contexts.push({ indent, kind: 'function', name: header.displayName, hostName: current?.hostName, line: lineIndex });
			}
		}
	}

	for (let index = contexts.length - 1; index >= 0; index -= 1) {
		const context = contexts[index];
		if (context.kind === 'function') {
			return context.line;
		}
	}

	return undefined;
}

function resolveExpressionType(document: TextDocument, position: Position, expression: string): string | undefined {
	const parts = expression.split('.').filter(Boolean);
	if (parts.length === 0) {
		return undefined;
	}

	let typeName = resolveNameType(document, position, parts[0]);
	for (const member of parts.slice(1)) {
		if (!typeName) {
			return undefined;
		}
		typeName = findMember(typeName, member)?.typeName ?? findMember(typeName, member)?.returnType;
	}

	return typeName;
}

function resolveNameType(document: TextDocument, position: Position, name: string): string | undefined {
	if (name === 'self') {
		return currentImplHost(document, position);
	}

	const local = visibleLocalSymbols(document, position).reverse().find(symbol => symbol.name === name);
	if (local?.typeName) {
		return local.typeName;
	}

	const symbol = findSymbol(name, document.uri);
	if (symbol?.kind === 'struct' || symbol?.kind === 'enum' || symbol?.kind === 'protocol') {
		return symbol.name;
	}

	return symbol?.typeName ?? symbol?.returnType;
}

function currentImplHost(document: TextDocument, position: Position): string | undefined {
	const lines = document.getText(Range.create(Position.create(0, 0), Position.create(position.line + 1, 0))).split(/\r?\n/);
	const contexts: BlockContext[] = [];
	const commentState: CommentState = { blockDepth: 0 };

	for (const rawLine of lines) {
		const codeLine = stripNonCode(rawLine, commentState);
		const trimmed = codeLine.trim();
		if (!trimmed) {
			continue;
		}

		const indent = measureIndent(rawLine);
		while (contexts.length > 0 && indent <= contexts[contexts.length - 1].indent) {
			contexts.pop();
		}

		const impl = parseImplDeclaration(trimmed);
		if (impl) {
			contexts.push({ indent, kind: 'impl', hostName: impl.host, name: impl.host });
			continue;
		}

		const current = contexts[contexts.length - 1];
		const header = parseFunctionHeader(trimmed, current?.hostName);
		if (header) {
			contexts.push({ indent, kind: 'function', name: header.name, hostName: current?.hostName });
		}
	}

	for (let index = contexts.length - 1; index >= 0; index -= 1) {
		const context = contexts[index];
		if (context.kind === 'impl' || context.hostName) {
			return context.hostName;
		}
	}

	return undefined;
}

function completionContext(document: TextDocument, position: Position): CompletionContext {
	const line = linePrefix(document, position);
	if (/^\s*use\s+(?:file\s+)?[\w'".\/-]*$/.test(line)) {
		return { useImport: true, shorthandVariant: false };
	}

	const memberMatch = line.match(/([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*)\.$/);
	if (memberMatch) {
		return { receiver: memberMatch[1], shorthandVariant: false, useImport: false };
	}

	if (line.endsWith('.')) {
		return { shorthandVariant: true, useImport: false };
	}

	return { shorthandVariant: false, useImport: false };
}

function tokenAtPosition(document: TextDocument, position: Position):
	| { text: string; range: Range }
	| undefined {
	const line = document.getText(Range.create(
		Position.create(position.line, 0),
		Position.create(position.line + 1, 0)
	));
	const regex = /\.?[A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*/g;
	let match: RegExpExecArray | null;

	while ((match = regex.exec(line)) !== null) {
		const start = match.index;
		const end = start + match[0].length;
		if (position.character >= start && position.character <= end) {
			return {
				text: match[0],
				range: Range.create(
					Position.create(position.line, start),
					Position.create(position.line, end)
				)
			};
		}
	}

	return undefined;
}

function memberAccessAtPosition(document: TextDocument, position: Position): { receiver: string; member: string } | undefined {
	const line = document.getText(Range.create(Position.create(position.line, 0), Position.create(position.line + 1, 0)));
	const regex = /([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*)\.([A-Za-z_]\w*)/g;
	let match: RegExpExecArray | null;

	while ((match = regex.exec(line)) !== null) {
		const start = match.index;
		const end = start + match[0].length;
		if (position.character >= start && position.character <= end) {
			return { receiver: match[1], member: match[2] };
		}
	}

	return undefined;
}

function linePrefix(document: TextDocument, position: Position): string {
	return document.getText(Range.create(
		Position.create(position.line, 0),
		position
	));
}

function readDocCommentLine(trimmedLine: string, inBlock: boolean): { text: string; continues: boolean } | undefined {
	if (inBlock) {
		const closes = trimmedLine.includes('*/');
		return {
			text: cleanupDocLine(trimmedLine.replace(/\*\//g, '')),
			continues: !closes
		};
	}

	if (trimmedLine.startsWith('///')) {
		return { text: trimmedLine.slice(3).trim(), continues: false };
	}

	if (trimmedLine.startsWith('//')) {
		return { text: trimmedLine.slice(2).trim(), continues: false };
	}

	if (trimmedLine.startsWith('/**') || trimmedLine.startsWith('/*')) {
		const closes = trimmedLine.includes('*/');
		return {
			text: cleanupDocLine(trimmedLine.replace(/^\/\*\*?/, '').replace(/\*\//g, '')),
			continues: !closes
		};
	}

	return undefined;
}

function cleanupDocLine(line: string): string {
	return line.replace(/^\s*\*\s?/, '').trim();
}

function splitOutsideDelimiters(text: string, delimiter: string): string[] {
	const parts: string[] = [];
	let current = '';
	let square = 0;
	let brace = 0;
	let paren = 0;
	let inString = false;

	for (let index = 0; index < text.length; index += 1) {
		const character = text[index];
		if (inString) {
			current += character;
			if (character === '\\') {
				index += 1;
				current += text[index] ?? '';
			} else if (character === "'") {
				inString = false;
			}
			continue;
		}

		if (character === "'") {
			inString = true;
			current += character;
			continue;
		}

		if (character === '[') square += 1;
		if (character === ']') square -= 1;
		if (character === '{') brace += 1;
		if (character === '}') brace -= 1;
		if (character === '(') paren += 1;
		if (character === ')') paren -= 1;

		if (character === delimiter && square === 0 && brace === 0 && paren === 0) {
			parts.push(current);
			current = '';
			continue;
		}

		current += character;
	}

	parts.push(current);
	return parts;
}

function splitWhitespaceOutsideDelimiters(text: string): string[] {
	const parts: string[] = [];
	let current = '';
	let square = 0;
	let brace = 0;
	let paren = 0;
	let inString = false;

	for (let index = 0; index < text.length; index += 1) {
		const character = text[index];
		if (inString) {
			current += character;
			if (character === '\\') {
				index += 1;
				current += text[index] ?? '';
			} else if (character === "'") {
				inString = false;
			}
			continue;
		}

		if (character === "'") {
			inString = true;
			current += character;
			continue;
		}

		if (character === '[') square += 1;
		if (character === ']') square -= 1;
		if (character === '{') brace += 1;
		if (character === '}') brace -= 1;
		if (character === '(') paren += 1;
		if (character === ')') paren -= 1;

		if (/\s/.test(character) && square === 0 && brace === 0 && paren === 0) {
			if (current) {
				parts.push(current);
				current = '';
			}
			continue;
		}

		current += character;
	}

	if (current) {
		parts.push(current);
	}

	return parts;
}

function splitTopLevelItems(text: string): string[] {
	const parts: string[] = [];
	let current = '';
	let square = 0;
	let brace = 0;
	let paren = 0;
	let inString = false;

	for (let index = 0; index < text.length; index += 1) {
		const character = text[index];
		if (inString) {
			current += character;
			if (character === '\\') {
				index += 1;
				current += text[index] ?? '';
			} else if (character === "'") {
				inString = false;
			}
			continue;
		}

		if (character === "'") {
			inString = true;
			current += character;
			continue;
		}

		if (character === '[') square += 1;
		if (character === ']') square -= 1;
		if (character === '{') brace += 1;
		if (character === '}') brace -= 1;
		if (character === '(') paren += 1;
		if (character === ')') paren -= 1;

		if ((/\s/.test(character) || character === ',') && square === 0 && brace === 0 && paren === 0) {
			if (current.trim()) {
				parts.push(current.trim());
				current = '';
			}
			continue;
		}

		current += character;
	}

	if (current.trim()) {
		parts.push(current.trim());
	}

	return parts;
}

function assignmentOperatorIndex(text: string): number {
	for (let index = 0; index < text.length; index += 1) {
		if (text[index] !== '=') {
			continue;
		}

		const previous = text[index - 1] ?? '';
		const next = text[index + 1] ?? '';
		if (next === '=' || ['+', '-', '*', '/', '!', '<', '>', '='].includes(previous)) {
			continue;
		}

		return index;
	}

	return -1;
}

function stripNonCode(line: string, state: CommentState): string {
	let output = '';
	let index = 0;
	let inString = false;

	while (index < line.length) {
		const current = line[index];
		const next = line[index + 1] ?? '';

		if (state.blockDepth > 0) {
			if (current === '/' && next === '*') {
				state.blockDepth += 1;
				output += '  ';
				index += 2;
			} else if (current === '*' && next === '/') {
				state.blockDepth -= 1;
				output += '  ';
				index += 2;
			} else {
				output += ' ';
				index += 1;
			}
			continue;
		}

		if (inString) {
			if (current === '\\') {
				output += ' ';
				if (index + 1 < line.length) {
					output += ' ';
				}
				index += 2;
				continue;
			}

			output += ' ';
			index += 1;
			if (current === "'") {
				inString = false;
			}
			continue;
		}

		if (current === '/' && next === '/') {
			output += ' '.repeat(line.length - index);
			break;
		}

		if (current === '/' && next === '*') {
			state.blockDepth += 1;
			output += '  ';
			index += 2;
			continue;
		}

		if (current === "'") {
			inString = true;
			output += ' ';
			index += 1;
			continue;
		}

		output += current;
		index += 1;
	}

	return output;
}

function stripCppLineComment(line: string): string {
	let inString: string | undefined;
	for (let index = 0; index < line.length - 1; index += 1) {
		const current = line[index];
		if (inString) {
			if (current === '\\') {
				index += 1;
			} else if (current === inString) {
				inString = undefined;
			}
			continue;
		}

		if (current === '"' || current === "'") {
			inString = current;
			continue;
		}

		if (current === '/' && line[index + 1] === '/') {
			return line.slice(0, index).trimEnd();
		}
	}

	return line;
}

function measureIndent(line: string): number {
	let indent = 0;

	for (const character of line) {
		if (character === ' ') {
			indent += 1;
		} else if (character === '\t') {
			indent += 4;
		} else {
			break;
		}
	}

	return indent;
}

function computeDiagnostics(text: string): Diagnostic[] {
	const diagnostics: Diagnostic[] = [];
	const bracketStack: Array<{ expected: string; line: number; character: number }> = [];
	const lines = text.split(/\r?\n/);
	let blockDepth = 0;
	let blockStart: Position | undefined;

	for (let lineIndex = 0; lineIndex < lines.length; lineIndex += 1) {
		const line = lines[lineIndex];
		let index = 0;
		let inString = false;
		let stringStart = 0;

		while (index < line.length) {
			const current = line[index];
			const next = line[index + 1] ?? '';

			if (blockDepth > 0) {
				if (current === '/' && next === '*') {
					blockDepth += 1;
					index += 2;
				} else if (current === '*' && next === '/') {
					blockDepth -= 1;
					index += 2;
				} else {
					index += 1;
				}
				continue;
			}

			if (inString) {
				if (current === '\\') {
					index += 2;
					continue;
				}

				if (current === "'") {
					inString = false;
				}

				index += 1;
				continue;
			}

			if (current === '/' && next === '/') {
				break;
			}

			if (current === '/' && next === '*') {
				blockDepth = 1;
				blockStart = Position.create(lineIndex, index);
				index += 2;
				continue;
			}

			if (current === "'") {
				inString = true;
				stringStart = index;
				index += 1;
				continue;
			}

			const expected = openingBracketExpectedClose(current);
			if (expected) {
				bracketStack.push({ expected, line: lineIndex, character: index });
				index += 1;
				continue;
			}

			if (isClosingBracket(current)) {
				const top = bracketStack[bracketStack.length - 1];
				if (top?.expected === current) {
					bracketStack.pop();
				} else {
					diagnostics.push(diagnostic(
						DiagnosticSeverity.Warning,
						lineIndex,
						index,
						index + 1,
						`Unmatched closing bracket '${current}'.`
					));
				}
			}

			index += 1;
		}

		if (inString) {
			diagnostics.push(diagnostic(
				DiagnosticSeverity.Warning,
				lineIndex,
				stringStart,
				line.length,
				'Unterminated string literal.'
			));
		}
	}

	if (blockDepth > 0 && blockStart) {
		diagnostics.push({
			severity: DiagnosticSeverity.Warning,
			range: Range.create(blockStart, Position.create(blockStart.line, blockStart.character + 2)),
			message: 'Unterminated block comment.',
			source: 'drast-lsp'
		});
	}

	for (const bracket of bracketStack) {
		diagnostics.push(diagnostic(
			DiagnosticSeverity.Warning,
			bracket.line,
			bracket.character,
			bracket.character + 1,
			`Unclosed bracket, expected '${bracket.expected}'.`
		));
	}

	diagnostics.push(...syntaxDiagnostics(text));
	return diagnostics;
}

function syntaxDiagnostics(text: string): Diagnostic[] {
	const diagnostics: Diagnostic[] = [];
	const lines = text.split(/\r?\n/);
	const commentState: CommentState = { blockDepth: 0 };

	for (let lineIndex = 0; lineIndex < lines.length; lineIndex += 1) {
		const rawLine = lines[lineIndex];
		const codeLine = stripNonCode(rawLine, commentState);
		const trimmed = codeLine.trim();
		if (!trimmed) {
			continue;
		}

		const leading = rawLine.length - rawLine.trimStart().length;
		if (/^fn\b/.test(trimmed)) {
			diagnostics.push(diagnostic(
				DiagnosticSeverity.Error,
				lineIndex,
				leading,
				leading + 2,
				'The fn operator was removed from Drast. Declare functions as `name param;Type, ReturnType`.'
			));
		}

		if (/^(let|var)\b/.test(trimmed)) {
			const keyword = trimmed.split(/\s+/)[0];
			diagnostics.push(diagnostic(
				DiagnosticSeverity.Warning,
				lineIndex,
				leading,
				leading + keyword.length,
				`Drast declarations do not use '${keyword}'. Use 'name Type = value' or 'name = value'.`
			));
		}

		for (const operator of unsupportedWordOperators) {
			const column = codeLine.search(new RegExp(`\\b${operator}\\b`));
			if (column >= 0) {
				diagnostics.push(diagnostic(
					DiagnosticSeverity.Error,
					lineIndex,
					column,
					column + operator.length,
					`'${operator}' is lexed by Drast but is not currently parsed as an expression operator.`
				));
			}
		}

		const useImport = trimmed.startsWith('use ') ? parseUseImport(trimmed) : undefined;
		if (trimmed === 'use' || (trimmed.startsWith('use ') && !useImport)) {
			diagnostics.push(diagnostic(
				DiagnosticSeverity.Error,
				lineIndex,
				leading,
				rawLine.length,
				'Malformed use statement.'
			));
		}

		if (trimmed.startsWith('use <')) {
			diagnostics.push(diagnostic(
				DiagnosticSeverity.Error,
				lineIndex,
				codeLine.indexOf('<'),
				codeLine.indexOf('<') + 1,
				'Angle include syntax is not supported. Use `use vector` or `use file header.hpp`.'
			));
		}

		if (/\bnothing\b/.test(trimmed) && trimmed !== 'nothing') {
			const column = codeLine.indexOf('nothing');
			diagnostics.push(diagnostic(
				DiagnosticSeverity.Warning,
				lineIndex,
				column,
				column + 'nothing'.length,
				'`nothing` is only valid as a standalone statement.'
			));
		}

		const firstWord = trimmed.split(/\s+/)[0];
		const currentIndent = measureIndent(rawLine);
		const functionBlock =
			(trimmed.includes(',') || trimmed.includes(';') || currentIndent === 0) &&
			parseFunctionHeader(trimmed) !== undefined;
		if (blockIntroducers.has(firstWord) || functionBlock) {
			const next = nextCodeLine(lines, lineIndex + 1);
			if (next && next.indent <= currentIndent && !trimmed.startsWith('use ') && !trimmed.startsWith('with ')) {
				diagnostics.push(diagnostic(
					DiagnosticSeverity.Warning,
					lineIndex,
					leading,
					rawLine.length,
					'Expected an indented Drast block after this line.'
				));
			}
		}
	}

	return diagnostics;
}

function nextCodeLine(lines: string[], start: number): { line: string; indent: number } | undefined {
	const state: CommentState = { blockDepth: 0 };
	for (let index = start; index < lines.length; index += 1) {
		const code = stripNonCode(lines[index], state).trim();
		if (code) {
			return { line: code, indent: measureIndent(lines[index]) };
		}
	}
	return undefined;
}

function diagnostic(severity: DiagnosticSeverity, line: number, start: number, end: number, message: string): Diagnostic {
	return {
		severity,
		range: Range.create(Position.create(line, start), Position.create(line, end)),
		message,
		source: 'drast-lsp'
	};
}

function openingBracketExpectedClose(character: string): string | undefined {
	if (character === '{') {
		return '}';
	}

	if (character === '[') {
		return ']';
	}

	if (character === '(') {
		return ')';
	}

	return undefined;
}

function isClosingBracket(character: string): boolean {
	return character === '}' || character === ']' || character === ')';
}

function formatDocumentEdits(document: TextDocument): TextEdit[] {
	const text = document.getText();
	const lines = text.split(/\r?\n/);
	const lineCount = text.endsWith('\n') || text.endsWith('\r\n') ? lines.length - 1 : lines.length;
	const edits: TextEdit[] = [];
	let blockCommentDepth = 0;

	for (let lineIndex = 0; lineIndex < lineCount; lineIndex += 1) {
		const original = lines[lineIndex] ?? '';
		const blockCommentState = blockCommentStateForLine(original, blockCommentDepth);
		const formatted = blockCommentState.touchesBlockComment ? original : formatLine(original);
		blockCommentDepth = blockCommentState.nextDepth;
		if (formatted !== original) {
			edits.push(TextEdit.replace(
				Range.create(Position.create(lineIndex, 0), Position.create(lineIndex, original.length)),
				formatted
			));
		}
	}

	return edits;
}

function formatDocumentText(text: string, _tabSize: number | undefined): string {
	const lines = text.split(/\r?\n/);
	const lineCount = text.endsWith('\n') || text.endsWith('\r\n') ? lines.length - 1 : lines.length;
	const output = lines.slice();
	let blockCommentDepth = 0;

	for (let lineIndex = 0; lineIndex < lineCount; lineIndex += 1) {
		const original = lines[lineIndex] ?? '';
		const blockCommentState = blockCommentStateForLine(original, blockCommentDepth);
		output[lineIndex] = blockCommentState.touchesBlockComment ? original : formatLine(original);
		blockCommentDepth = blockCommentState.nextDepth;
	}

	return output.join(text.includes('\r\n') ? '\r\n' : '\n');
}

function blockCommentStateForLine(line: string, startingDepth: number): { nextDepth: number; touchesBlockComment: boolean } {
	let depth = startingDepth;
	let touchesBlockComment = depth > 0;
	let inString = false;

	for (let index = 0; index < line.length - 1; index += 1) {
		const current = line[index];
		const next = line[index + 1];

		if (inString) {
			if (current === '\\') {
				index += 1;
			} else if (current === "'") {
				inString = false;
			}
			continue;
		}

		if (current === "'") {
			inString = true;
			continue;
		}

		if (depth === 0 && current === '/' && next === '/') {
			break;
		}

		if (current === '/' && next === '*') {
			depth += 1;
			touchesBlockComment = true;
			index += 1;
			continue;
		}

		if (depth > 0 && current === '*' && next === '/') {
			depth -= 1;
			touchesBlockComment = true;
			index += 1;
		}
	}

	return { nextDepth: depth, touchesBlockComment };
}

function formatLine(line: string): string {
	if (!line.trim()) {
		return '';
	}

	const comment = splitInlineComment(line);
	if (comment.comment && !comment.code.trim()) {
		return `${comment.code}${comment.comment.trimStart()}`;
	}

	const normalizedCode = normalizeCodeLine(comment.code).replace(/[ \t]+$/g, '');
	if (!comment.comment) {
		return normalizedCode;
	}

	const commentText = comment.comment.trimStart();
	if (!normalizedCode.trim()) {
		return `${normalizedCode}${commentText}`;
	}

	return `${normalizedCode} ${commentText}`;
}

function shouldInsertTopLevelBlank(output: string[], normalized: string, indentLevel: number): boolean {
	if (indentLevel !== 0 || output.length === 0 || output[output.length - 1] === '') {
		return false;
	}

	if (normalized.startsWith('use ') || output[output.length - 1].startsWith('use ')) {
		return false;
	}

	return isTopLevelDeclarationLine(normalized) && isTopLevelDeclarationLine(output[output.length - 1].trim());
}

function isTopLevelDeclarationLine(line: string): boolean {
	return Boolean(
		parseTypeDeclaration(line) ||
		parseImplDeclaration(line) ||
		parseFunctionHeader(line) ||
		parseVariableDeclaration(line, line)
	);
}

function normalizeCodeLine(line: string): string {
	return transformOutsideStrings(line, segment => normalizeCodeSegment(segment));
}

function normalizeCodeSegment(segment: string): string {
	const placeholders: string[] = [];
	let out = segment.replace(/operator\[[^\]]+\]/g, match => {
		const key = `__DRAST_OPERATOR_${placeholders.length}__`;
		placeholders.push(match);
		return key;
	});

	out = out.replace(/[ \t]*(\+=|-=|\*=|\/=|==)[ \t]*/g, ' $1 ');
	out = out.replace(/[ \t]*(?<![!<>=+\-*/])=(?![=>])[ \t]*/g, ' = ');
	out = out.replace(/[ \t]*,[ \t]*/g, ', ');

	for (let index = 0; index < placeholders.length; index += 1) {
		out = out.replace(`__DRAST_OPERATOR_${index}__`, placeholders[index]);
	}

	return out;
}

function transformOutsideStrings(text: string, transform: (segment: string) => string): string {
	let output = '';
	let segment = '';
	let index = 0;

	while (index < text.length) {
		const character = text[index];
		if (character !== "'") {
			segment += character;
			index += 1;
			continue;
		}

		output += transform(segment);
		segment = '';
		let stringLiteral = character;
		index += 1;
		while (index < text.length) {
			const current = text[index];
			stringLiteral += current;
			index += 1;
			if (current === '\\' && index < text.length) {
				stringLiteral += text[index];
				index += 1;
				continue;
			}
			if (current === "'") {
				break;
				}
			}
			if (shouldInsertSpaceBeforeString(output)) {
				output += ' ';
			}
			output += stringLiteral;
	}

	output += transform(segment);
	return output;
}

function shouldInsertSpaceBeforeString(text: string): boolean {
	if (!/[A-Za-z_0-9\]\}]$/.test(text)) {
		return false;
	}

	return !/(^|[^A-Za-z_0-9])s$/.test(text);
}

function splitInlineComment(line: string): { code: string; comment?: string } {
	let inString = false;
	for (let index = 0; index < line.length - 1; index += 1) {
		const character = line[index];
		if (inString) {
			if (character === '\\') {
				index += 1;
			} else if (character === "'") {
				inString = false;
			}
			continue;
		}

		if (character === "'") {
			inString = true;
			continue;
		}

		if (character === '/' && line[index + 1] === '/') {
			return {
				code: line.slice(0, index),
				comment: line.slice(index)
			};
		}
	}

	return { code: line };
}

function fullDocumentRange(document: TextDocument): Range {
	const text = document.getText();
	const end = document.positionAt(text.length);
	return Range.create(Position.create(0, 0), end);
}

function uriToFsPath(uri: string): string | undefined {
	try {
		return fileURLToPath(uri);
	} catch {
		return undefined;
	}
}

function uriDisplayName(uri: string): string {
	if (uri.startsWith('drast:')) {
		return uri;
	}

	const fsPath = uriToFsPath(uri);
	return fsPath ? path.basename(fsPath) : uri;
}
