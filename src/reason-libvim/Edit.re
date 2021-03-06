open EditorCoreTypes;

module Log = (val Timber.Log.withNamespace("SingleEdit"));

[@deriving show]
type t = {
  range: CharacterRange.t,
  text: array(string),
};

type editResult = {
  oldStartLine: LineNumber.t,
  oldEndLine: LineNumber.t,
  newLines: array(string),
};

module Internal = {
  let applyPrefix = (~prefix, lines) =>
    if (String.length(prefix) == 0) {
      lines;
    } else {
      let lines' = Array.length(lines) == 0 ? [|""|] : Array.copy(lines);
      let firstLine = lines'[0];
      lines'[0] = prefix ++ firstLine;
      lines';
    };

  let applyPostfix = (~postfix, lines) =>
    if (String.length(postfix) == 0) {
      lines;
    } else {
      let lines' = Array.length(lines) == 0 ? [|""|] : Array.copy(lines);
      let len = Array.length(lines');
      let lastLine = lines'[len - 1];
      lines'[len - 1] = lastLine ++ postfix;
      lines';
    };
};

let applyEdit = (~provider, edit) => {
  // For now, we assume the start line and end line are the same
  // this is safe for formatting operations, but we'll need to
  // improve this for more general edits (like code actions).
  let startLine =
    edit.range.start.line |> EditorCoreTypes.LineNumber.toZeroBased;
  let endLine = edit.range.stop.line |> EditorCoreTypes.LineNumber.toZeroBased;
  let startColumn = edit.range.start.character |> CharacterIndex.toInt;
  let endColumn = edit.range.stop.character |> CharacterIndex.toInt;

  try({
    let prefix =
      startColumn == 0
        ? ""
        : startLine
          |> provider
          |> Option.map(str => Zed_utf8.sub(str, 0, startColumn))
          |> Option.value(~default="");

    let postfix =
      endLine
      |> provider
      |> Option.map(str =>
           Zed_utf8.sub(str, endColumn, Zed_utf8.length(str) - endColumn)
         )
      |> Option.value(~default="");

    let lines' =
      edit.text
      |> Internal.applyPrefix(~prefix)
      |> Internal.applyPostfix(~postfix);

    Ok({
      oldStartLine: edit.range.start.line,
      oldEndLine: edit.range.stop.line,
      newLines: lines',
    });
  }) {
  | exn => Error(Printexc.to_string(exn))
  };
};

let sort = edits => {
  let sortFn = (editA, editB) => {
    let rangeA = editA.range;
    let rangeB = editB.range;

    let startLineA =
      rangeA.start.line |> EditorCoreTypes.LineNumber.toZeroBased;
    let startLineB =
      rangeB.start.line |> EditorCoreTypes.LineNumber.toZeroBased;

    let startColumnA = rangeA.start.character |> CharacterIndex.toInt;
    let startColumnB = rangeB.start.character |> CharacterIndex.toInt;

    if (startLineA == startLineB) {
      startColumnB - startColumnA;
    } else {
      startLineB - startLineA;
    };
  };
  List.sort(sortFn, edits);
};
