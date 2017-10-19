program fpc_tracer;

{$mode delphi}// {$H+}

uses
  {$IFDEF UNIX}{$IFDEF UseCThreads}
  cthreads,
  {$ENDIF}{$ENDIF}
  Classes, SysUtils, CustApp,
  ctypes,fncs;

type

  { TFpcTracer }

  TFpcTracer = class(TCustomApplication)
  protected
    procedure DoRun; override;
  public
    constructor Create(TheOwner: TComponent); override;
    destructor Destroy; override;
    procedure WriteHelp; virtual;
  end;

{ TFpcTracer }

procedure TFpcTracer.DoRun;
var
  ErrorMsg: String;
  time_granted, time_stop: fncs_time;
  events: ppchar;
  i: integer;
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
  time_granted := 0;
  time_stop := 100;
  fncs_initialize;

  while time_granted < time_stop do begin
	  time_granted := fncs_time_request(time_stop);
  	events := fncs_get_events();
    i := 0;
    while events[i] <> nil do begin
      writeln ('t=',time_granted, ' ', events[i], '=', fncs_get_value(events[i]));
      inc(i);
    end;
  end;

  fncs_finalize;

  // stop program loop
  Terminate;
end;

constructor TFpcTracer.Create(TheOwner: TComponent);
begin
  inherited Create(TheOwner);
  StopOnException:=True;
end;

destructor TFpcTracer.Destroy;
begin
  inherited Destroy;
end;

procedure TFpcTracer.WriteHelp;
begin
  { add your help code here }
  writeln('Usage: ', ExeName, ' -h');
end;

var
  Application: TFpcTracer;
begin
  Application:=TFpcTracer.Create(nil);
  Application.Title:='FPC FNCS Tracer';
  Application.Run;
  Application.Free;
end.

