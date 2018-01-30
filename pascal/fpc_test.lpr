program fpc_test;

{$mode objfpc}{$H+}

uses
  {$IFDEF UNIX}{$IFDEF UseCThreads}
  cthreads,
  {$ENDIF}{$ENDIF}
  Classes, SysUtils, CustApp,
  ctypes, fncs;

type

  { TFpcTest }

  TFpcTest = class(TCustomApplication)
  protected
    procedure DoRun; override;
  public
    constructor Create(TheOwner: TComponent); override;
    destructor Destroy; override;
    procedure WriteHelp; virtual;
  end;

{ TFpcTest }

procedure TFpcTest.DoRun;
var
  ErrorMsg: String;
  current_time, next_time, time_stop: fncs_time;
  config: string;
begin
  // quick check parameters
  ErrorMsg:=CheckOptions('h', 'help');
  if ErrorMsg<>'' then begin
    ShowException(Exception.Create(ErrorMsg));
    Terminate;
    Exit;
  end;

  // parse parameters
  if HasOption('h', 'help') then begin
    WriteHelp;
    Terminate;
    Exit;
  end;

  { add your program here }
  time_stop := 100;
  config := 'name=fpc_test' + chr(13) + chr(10) + 'time_delta=1s';
  writeln(config);
  fncs_initialize_config (PChar (config));

  for next_time := 1 to time_stop do begin
    current_time := fncs_time_request (next_time);
    fncs_publish ('some_key', PChar (IntToStr (current_time * 2)));
  end;

  fncs_finalize;

  // stop program loop
  Terminate;
end;

constructor TFpcTest.Create(TheOwner: TComponent);
begin
  inherited Create(TheOwner);
  StopOnException:=True;
end;

destructor TFpcTest.Destroy;
begin
  inherited Destroy;
end;

procedure TFpcTest.WriteHelp;
begin
  { add your help code here }
  writeln('Usage: ', ExeName, ' -h');
end;

var
  Application: TFpcTest;
begin
  Application:=TFpcTest.Create(nil);
  Application.Title:='FPC FNCS Test';
  Application.Run;
  Application.Free;
end.

