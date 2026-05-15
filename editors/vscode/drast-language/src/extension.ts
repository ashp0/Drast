import * as path from 'path';
import * as vscode from 'vscode';
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient | undefined;

type DrastDocumentFilter = { scheme: string; language: string };

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
  shl: 'Bitwise shift-left token, currently lexed but not parsed as an expression operator.',
  shr: 'Bitwise shift-right token, currently lexed but not parsed as an expression operator.',
  bor: 'Bitwise or token, currently lexed but not parsed as an expression operator.',
  band: 'Bitwise and token, currently lexed but not parsed as an expression operator.',
  bxor: 'Bitwise xor token, currently lexed but not parsed as an expression operator.',
  nothing: 'Placeholder keyword for an intentionally empty block.'
};

const snippetSpecs = [
  {
    label: 'main',
    insertText: 'main, int\n\t${1:return 0}',
    detail: 'Drast entry point snippet'
  },
  {
    label: 'function',
    insertText: '${1:name} ${2:param};${3:int}, ${4:int}\n\t${5:return ${2}}',
    detail: 'Drast function snippet'
  },
  {
    label: 'struct',
    insertText: 'struct ${1:Name}\n\t${2:field} ${3:int}',
    detail: 'Drast struct snippet'
  },
  {
    label: 'enum',
    insertText: 'enum ${1:Name}\n\t${2:First}; ${3:Second}; ${4:Third};',
    detail: 'Drast enum snippet'
  },
  {
    label: 'impl',
    insertText: 'impl ${1:Type}\n\t${2:method}, ${3:void}\n\t\t${4:nothing}',
    detail: 'Drast impl snippet'
  },
  {
    label: 'protocol',
    insertText: 'protocol ${1:Name}\n\t${2:method}, ${3:void}',
    detail: 'Drast protocol snippet'
  },
  {
    label: 'match',
    insertText: 'match ${1:value}\n\t${2:Pattern}\n\t\t${3:nothing}\n\tdefault\n\t\t${4:nothing}',
    detail: 'Drast match snippet'
  },
  {
    label: 'if',
    insertText: 'if ${1:condition}\n\t${2:body}',
    detail: 'Drast if snippet'
  },
  {
    label: 'while',
    insertText: 'while ${1:condition}\n\t${2:body}',
    detail: 'Drast while snippet'
  },
  {
    label: 'for',
    insertText: 'for ${1:item} in ${2:collection}\n\t${3:body}',
    detail: 'Drast for snippet'
  },
  {
    label: 'try catch',
    insertText: 'try\n\t${1:nothing}\ncatch ;${2:error}\n\t${3:println ${2}}',
    detail: 'Drast try/catch snippet'
  },
  {
    label: 'comptime',
    insertText: 'comptime\n\t${1:nothing}',
    detail: 'Drast comptime snippet'
  },
  {
    label: 'nothing',
    insertText: 'nothing',
    detail: 'Drast empty-block keyword'
  }
];

export async function activate(context: vscode.ExtensionContext): Promise<void> {
  const selector: DrastDocumentFilter[] = [
    { scheme: 'file', language: 'drast' },
    { scheme: 'untitled', language: 'drast' }
  ];

  context.subscriptions.push(registerFallbackCompletions(selector));

  context.subscriptions.push(
    vscode.commands.registerCommand('drast.restartServer', async () => {
      await restartClient(context, selector);
      void vscode.window.showInformationMessage('Drast language server restarted.');
    })
  );

  await startClient(context, selector);
}

export async function deactivate(): Promise<void> {
  if (client) {
    await client.stop();
    client.dispose();
    client = undefined;
  }
}

function registerFallbackCompletions(selector: DrastDocumentFilter[]): vscode.Disposable {
  return vscode.languages.registerCompletionItemProvider(selector as vscode.DocumentSelector, {
    provideCompletionItems(): vscode.CompletionItem[] {
      const keywordItems = Object.entries(keywordDescriptions).map(([keyword, description]) => {
        const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
        item.detail = 'Drast keyword';
        item.documentation = new vscode.MarkdownString(description);
        return item;
      });

      const snippets = snippetSpecs.map((snippet) => {
        const item = new vscode.CompletionItem(snippet.label, vscode.CompletionItemKind.Snippet);
        item.detail = snippet.detail;
        item.insertText = new vscode.SnippetString(snippet.insertText);
        item.documentation = new vscode.MarkdownString(`Expands to:\n\n\`\`\`drast\n${snippet.insertText}\n\`\`\``);
        return item;
      });

      return [...snippets, ...keywordItems];
    }
  });
}

async function restartClient(context: vscode.ExtensionContext, selector: DrastDocumentFilter[]): Promise<void> {
  if (client) {
    await client.stop();
    client.dispose();
    client = undefined;
  }

  await startClient(context, selector);
}

async function startClient(context: vscode.ExtensionContext, selector: DrastDocumentFilter[]): Promise<void> {
  const serverModule = context.asAbsolutePath(path.join('out', 'server.js'));
  const serverOptions: ServerOptions = {
    run: {
      module: serverModule,
      transport: TransportKind.ipc
    },
    debug: {
      module: serverModule,
      transport: TransportKind.ipc,
      options: {
        execArgv: ['--nolazy', '--inspect=6009']
      }
    }
  };

  const clientOptions: LanguageClientOptions = {
    documentSelector: selector,
    synchronize: {
      fileEvents: vscode.workspace.createFileSystemWatcher('**/*.drast')
    }
  };

  client = new LanguageClient(
    'drastLanguageServer',
    'Drast Language Server',
    serverOptions,
    clientOptions
  );

  context.subscriptions.push(client);
  await client.start();
}
