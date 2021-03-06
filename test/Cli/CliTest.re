open TestFramework;

let noenv = _ => None;

describe("CLI", ({describe, test, _}) => {
  test("no files, no folders", ({expect, _}) => {
    let (options, eff) = Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor"|]);
    expect.equal(eff, Run);
    expect.equal(options.folder, None);
    expect.equal(options.filesToOpen, []);
  });
  describe("folder tests", ({test, _}) => {
    test(". should be a folder", ({expect, _}) => {
      let (options, eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "."|]);
      expect.equal(eff, Run);
      expect.equal(options.folder != None, true);
      expect.equal(options.filesToOpen, []);
    });

    test(".. should be a folder", ({expect, _}) => {
      let (options, eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", ".."|]);
      expect.equal(eff, Run);
      expect.equal(options.folder != None, true);
      expect.equal(options.filesToOpen, []);
    });
  });
  describe("file tests", ({test, _}) => {
    // NOTE: This test relies on the working directory being the repo root
    test("README.md should be a file", ({expect, _}) => {
      let (options, eff) =
        Oni_CLI.parse(~getenv=noenv, [|"Oni2_editor", "README.md"|]);
      expect.equal(eff, Run);
      expect.equal(options.filesToOpen |> List.length, 1);
    })
  });
  describe("syntax server", ({test, _}) => {
    test("Syntax server with PID", ({expect, _}) => {
      let (_options, eff) =
        Oni_CLI.parse(
          ~getenv=noenv,
          [|"Oni2_editor", "--syntax-highlight-service", "1234:named-pipe"|],
        );
      expect.equal(
        eff,
        StartSyntaxServer({parentPid: "1234", namedPipe: "named-pipe"}),
      );
    })
  });
});
