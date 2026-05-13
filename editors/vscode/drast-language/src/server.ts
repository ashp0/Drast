import {
  CompletionItem,
  CompletionItemKind,
  createConnection,
  Diagnostic,
  DiagnosticSeverity,
  Hover,
  InitializeParams,
  InitializeResult,
  InsertTextFormat,
  Location,
  MarkupKind,
  Position,
  ProposedFeatures,
  Range,
  TextDocumentPositionParams,
  TextDocumentSyncKind,
  TextDocuments
} from 'vscode-languageserver/node';
import { TextDocument } from 'vscode-languageserver-textdocument';
import * as fs from 'fs/promises';
import * as path from 'path';
import { fileURLToPath, pathToFileURL } from 'url';

type SymbolKind = 'function' | 'variable' | 'class' | 'enum' | 'interface';

interface DrastSymbol {
  name: string;
  kind: SymbolKind;
  completionKind: CompletionItemKind;
  uri: string;
  range: Range;
  selectionRange: Range;
  definitionLine: string;
  detail: string;
}

interface BlockContext {
  indent: number;
  kind: 'struct' | 'enum' | 'impl' | 'protocol' | 'function';
}

interface CommentState {
  blockDepth: number;
}

const connection = createConnection(ProposedFeatures.all);
const documents = new TextDocuments(TextDocument);

let workspaceRootUris: string[] = [];
let workspaceScanPromise: Promise<void> | undefined;
let hasScannedWorkspace = false;

const documentSymbols = new Map<string, DrastSymbol[]>();
const symbolTable = new Map<string, DrastSymbol[]>();

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
  enum: 'Declare a simple enum class or data enum.',
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
  shl: 'Bitwise shift-left token, currently lexed but not parsed as an expression operator.',
  shr: 'Bitwise shift-right token, currently lexed but not parsed as an expression operator.',
  bor: 'Bitwise or token, currently lexed but not parsed as an expression operator.',
  band: 'Bitwise and token, currently lexed but not parsed as an expression operator.',
  bxor: 'Bitwise xor token, currently lexed but not parsed as an expression operator.',
  nothing: 'Placeholder keyword for an intentionally empty block.'
};

const declarationKeywords = new Set([
  'use',
  'return',
  'if',
  'elif',
  'else',
  'while',
  'for',
  'break',
  'continue',
  'in',
  'to',
  'until',
  'step',
  'struct',
  'enum',
  'impl',
  'protocol',
  'match',
  'default',
  'with',
  'as',
  'try',
  'catch',
  'private',
  'preview',
  'fileprivate',
  'discard',
  'operator',
  'maybe',
  'true',
  'false',
  'nil',
  'self',
  'variadic',
  'tuple',
  'and',
  'or',
  'not',
  'nothing'
]);

const snippetCompletions: CompletionItem[] = [
  {
    label: 'if',
    kind: CompletionItemKind.Snippet,
    detail: 'Drast if snippet',
    insertText: 'if ${1:condition}\n\t${2:body}',
    insertTextFormat: InsertTextFormat.Snippet
  },
  {
    label: 'while',
    kind: CompletionItemKind.Snippet,
    detail: 'Drast while snippet',
    insertText: 'while ${1:condition}\n\t${2:body}',
    insertTextFormat: InsertTextFormat.Snippet
  },
  {
    label: 'for',
    kind: CompletionItemKind.Snippet,
    detail: 'Drast for snippet',
    insertText: 'for ${1:item} in ${2:collection}\n\t${3:body}',
    insertTextFormat: InsertTextFormat.Snippet
  },
  {
    label: 'nothing',
    kind: CompletionItemKind.Snippet,
    detail: 'Drast empty-block keyword',
    insertText: 'nothing',
    insertTextFormat: InsertTextFormat.Snippet
  }
];

connection.onInitialize((params: InitializeParams): InitializeResult => {
  workspaceRootUris = collectWorkspaceRoots(params);

  return {
    capabilities: {
      textDocumentSync: TextDocumentSyncKind.Incremental,
      completionProvider: {
        resolveProvider: false,
        triggerCharacters: ['.', '`']
      },
      hoverProvider: true,
      definitionProvider: true
    }
  };
});

connection.onInitialized(() => {
  void scheduleWorkspaceScan();
});

connection.onDidChangeWatchedFiles(() => {
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

connection.onCompletion(async (): Promise<CompletionItem[]> => {
  await ensureWorkspaceScanned();
  return [
    ...snippetCompletions,
    ...keywordCompletionItems(),
    ...symbolCompletionItems()
  ];
});

connection.onHover(async (params: TextDocumentPositionParams): Promise<Hover | null> => {
  await ensureWorkspaceScanned();

  const document = documents.get(params.textDocument.uri);
  if (!document) {
    return null;
  }

  const word = wordAtPosition(document, params.position);
  if (!word) {
    return null;
  }

  const keywordDescription = keywordDescriptions[word.text];
  if (keywordDescription) {
    return {
      contents: {
        kind: MarkupKind.Markdown,
        value: `**${word.text}**\n\n${keywordDescription}`
      },
      range: word.range
    };
  }

  const symbol = findSymbol(word.text, params.textDocument.uri);
  if (!symbol) {
    return null;
  }

  return {
    contents: {
      kind: MarkupKind.Markdown,
      value: `**${symbol.name}** (${symbol.detail})\n\n\`${symbol.definitionLine.trim()}\``
    },
    range: word.range
  };
});

connection.onDefinition(async (params: TextDocumentPositionParams): Promise<Location | null> => {
  await ensureWorkspaceScanned();

  const document = documents.get(params.textDocument.uri);
  if (!document) {
    return null;
  }

  const word = wordAtPosition(document, params.position);
  if (!word || keywordDescriptions[word.text]) {
    return null;
  }

  const symbol = findSymbol(word.text, params.textDocument.uri);
  if (!symbol) {
    return null;
  }

  return Location.create(symbol.uri, symbol.selectionRange);
});

documents.listen(connection);
connection.listen();

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

function symbolCompletionItems(): CompletionItem[] {
  const seen = new Set<string>();
  const items: CompletionItem[] = [];

  for (const symbols of symbolTable.values()) {
    for (const symbol of symbols) {
      const key = `${symbol.kind}:${symbol.name}`;
      if (seen.has(key) || keywordDescriptions[symbol.name]) {
        continue;
      }

      seen.add(key);
      items.push({
        label: symbol.name,
        kind: symbol.completionKind,
        detail: symbol.detail,
        documentation: {
          kind: MarkupKind.Markdown,
          value: `Defined in \`${uriDisplayName(symbol.uri)}\`:\n\n\`${symbol.definitionLine.trim()}\``
        }
      });
    }
  }

  return items.sort((left, right) => left.label.localeCompare(right.label));
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

    rebuildSymbolTable();
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
  rebuildSymbolTable();
  connection.sendDiagnostics({
    uri: document.uri,
    diagnostics: computeDiagnostics(document.getText())
  });
}

function rebuildSymbolTable(): void {
  symbolTable.clear();

  for (const symbols of documentSymbols.values()) {
    for (const symbol of symbols) {
      const existing = symbolTable.get(symbol.name) ?? [];
      existing.push(symbol);
      symbolTable.set(symbol.name, existing);
    }
  }
}

function scanSymbolsInText(uri: string, text: string): DrastSymbol[] {
  const symbols: DrastSymbol[] = [];
  const lines = text.split(/\r?\n/);
  const contexts: BlockContext[] = [];
  const commentState: CommentState = { blockDepth: 0 };

  for (let lineIndex = 0; lineIndex < lines.length; lineIndex += 1) {
    const rawLine = lines[lineIndex];
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
    const topLevel = contexts.length === 0;

    const typeDeclaration = typeDeclarationMatch(codeLine);
    if (typeDeclaration) {
      symbols.push(makeSymbol(
        typeDeclaration.name,
        typeDeclaration.kind,
        typeDeclaration.completionKind,
        uri,
        lineIndex,
        codeLine.indexOf(typeDeclaration.name),
        rawLine,
        typeDeclaration.detail
      ));

      contexts.push({ indent, kind: typeDeclaration.contextKind });
      continue;
    }

    const implDeclaration = codeLine.match(/^\s*impl\s+([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)?)(?:\s+as\s+[A-Za-z_]\w*)?/);
    if (implDeclaration) {
      contexts.push({ indent, kind: 'impl' });
      continue;
    }

    const legacyFunction = codeLine.match(/^\s*fn\s+([A-Za-z_]\w*)\b/);
    if (legacyFunction) {
      const name = legacyFunction[1];
      symbols.push(makeSymbol(
        name,
        'function',
        CompletionItemKind.Function,
        uri,
        lineIndex,
        codeLine.indexOf(name),
        rawLine,
        'function'
      ));
      contexts.push({ indent, kind: 'function' });
      continue;
    }

    const shouldScanFunction =
      topLevel ||
      (currentContext !== undefined &&
        (currentContext.kind === 'impl' || currentContext.kind === 'protocol') &&
        indent > currentContext.indent);

    if (shouldScanFunction) {
      const functionName = functionHeaderName(trimmed);
      if (functionName) {
        symbols.push(makeSymbol(
          functionName,
          'function',
          CompletionItemKind.Function,
          uri,
          lineIndex,
          codeLine.indexOf(functionName.replace(/^operator/, 'operator')),
          rawLine,
          'function'
        ));
        contexts.push({ indent, kind: 'function' });
        continue;
      }
    }

    if (topLevel) {
      const variableName = variableDeclarationName(codeLine);
      if (variableName) {
        symbols.push(makeSymbol(
          variableName.name,
          'variable',
          CompletionItemKind.Variable,
          uri,
          lineIndex,
          variableName.character,
          rawLine,
          variableName.detail
        ));
      }
    }
  }

  return symbols;
}

function typeDeclarationMatch(codeLine: string):
  | {
      name: string;
      kind: SymbolKind;
      completionKind: CompletionItemKind;
      detail: string;
      contextKind: BlockContext['kind'];
    }
  | undefined {
  const match = codeLine.match(/^\s*(?:fileprivate\s+)?(struct|enum|protocol)\s+([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)?)/);
  if (!match) {
    return undefined;
  }

  const declarationKind = match[1];
  const name = match[2];

  if (declarationKind === 'struct') {
    return {
      name,
      kind: 'class',
      completionKind: CompletionItemKind.Class,
      detail: 'struct',
      contextKind: 'struct'
    };
  }

  if (declarationKind === 'enum') {
    return {
      name,
      kind: 'enum',
      completionKind: CompletionItemKind.Enum,
      detail: 'enum',
      contextKind: 'enum'
    };
  }

  return {
    name,
    kind: 'interface',
    completionKind: CompletionItemKind.Interface,
    detail: 'protocol',
    contextKind: 'protocol'
  };
}

function functionHeaderName(trimmedLine: string): string | undefined {
  if (assignmentOperatorIndex(trimmedLine) >= 0) {
    return undefined;
  }

  if (/;\s*$/.test(trimmedLine) && !trimmedLine.includes('operator[')) {
    return undefined;
  }

  const operatorMatch = trimmedLine.match(/^(?:private\s+|preview\s+)?operator\[([^\]]+)\]/);
  if (operatorMatch) {
    return `operator${operatorMatch[1]}`;
  }

  const match = trimmedLine.match(/^(?:private\s+|preview\s+)?([A-Za-z_]\w*)\b/);
  if (!match) {
    return undefined;
  }

  const name = match[1];
  if (declarationKeywords.has(name)) {
    return undefined;
  }

  return name;
}

function variableDeclarationName(codeLine: string): { name: string; character: number; detail: string } | undefined {
  const legacy = codeLine.match(/^\s*(?:let|var|const)\s+([A-Za-z_]\w*)\b/);
  if (legacy) {
    return {
      name: legacy[1],
      character: codeLine.indexOf(legacy[1]),
      detail: 'variable'
    };
  }

  const equalIndex = assignmentOperatorIndex(codeLine);
  if (equalIndex < 0) {
    return undefined;
  }

  const beforeEqual = codeLine.slice(0, equalIndex).trim();
  if (beforeEqual.length === 0 || beforeEqual.includes('.') || beforeEqual.includes('[')) {
    return undefined;
  }

  const match = beforeEqual.match(/^([A-Za-z_]\w*)\b/);
  if (!match || declarationKeywords.has(match[1])) {
    return undefined;
  }

  return {
    name: match[1],
    character: codeLine.indexOf(match[1]),
    detail: beforeEqual.includes('const') ? 'constant' : 'variable'
  };
}

function assignmentOperatorIndex(text: string): number {
  for (let index = 0; index < text.length; index += 1) {
    if (text[index] !== '=') {
      continue;
    }

    const previous = text[index - 1] ?? '';
    const next = text[index + 1] ?? '';
    if (next === '=' || ['+', '-', '*', '/', '!', '<', '>'].includes(previous)) {
      continue;
    }

    return index;
  }

  return -1;
}

function makeSymbol(
  name: string,
  kind: SymbolKind,
  completionKind: CompletionItemKind,
  uri: string,
  line: number,
  character: number,
  definitionLine: string,
  detail: string
): DrastSymbol {
  const safeCharacter = Math.max(0, character);
  const range = Range.create(
    Position.create(line, 0),
    Position.create(line, definitionLine.length)
  );
  const selectionRange = Range.create(
    Position.create(line, safeCharacter),
    Position.create(line, safeCharacter + name.length)
  );

  return {
    name,
    kind,
    completionKind,
    uri,
    range,
    selectionRange,
    definitionLine,
    detail
  };
}

function findSymbol(name: string, preferredUri: string): DrastSymbol | undefined {
  const symbols = symbolTable.get(name);
  if (!symbols || symbols.length === 0) {
    return undefined;
  }

  return symbols.find((symbol) => symbol.uri === preferredUri) ?? symbols[0];
}

function wordAtPosition(document: TextDocument, position: Position):
  | { text: string; range: Range }
  | undefined {
  const line = document.getText(Range.create(
    Position.create(position.line, 0),
    Position.create(position.line + 1, 0)
  ));
  const wordRegex = /[A-Za-z_]\w*/g;
  let match: RegExpExecArray | null;

  while ((match = wordRegex.exec(line)) !== null) {
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
          diagnostics.push(warningDiagnostic(
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
      diagnostics.push(warningDiagnostic(
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
    diagnostics.push(warningDiagnostic(
      bracket.line,
      bracket.character,
      bracket.character + 1,
      `Unclosed bracket, expected '${bracket.expected}'.`
    ));
  }

  return diagnostics;
}

function warningDiagnostic(line: number, start: number, end: number, message: string): Diagnostic {
  return {
    severity: DiagnosticSeverity.Warning,
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

function uriToFsPath(uri: string): string | undefined {
  try {
    return fileURLToPath(uri);
  } catch {
    return undefined;
  }
}

function uriDisplayName(uri: string): string {
  const fsPath = uriToFsPath(uri);
  return fsPath ? path.basename(fsPath) : uri;
}
