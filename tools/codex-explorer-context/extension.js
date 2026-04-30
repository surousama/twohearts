const vscode = require("vscode");

function activate(context) {
  const disposable = vscode.commands.registerCommand(
    "codexExplorerContext.addFileToCodexThread",
    async (resource) => {
      if (!resource || resource.scheme !== "file") {
        vscode.window.showWarningMessage("请选择一个本地文件后再发送到 Codex。");
        return;
      }

      try {
        await vscode.commands.executeCommand("chatgpt.openSidebar");
        await vscode.commands.executeCommand("chatgpt.addFileToThread", resource);
      } catch (error) {
        const message =
          error instanceof Error ? error.message : "未知错误";

        vscode.window.showErrorMessage(
          `调用 Codex 失败：${message}。请确认官方 Codex 扩展已安装并已登录。`
        );
      }
    }
  );

  context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = {
  activate,
  deactivate
};
