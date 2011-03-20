using Gtk;

void
main(string[] args)
{
	init(ref args);
	var mainwin = new MainWin();
	mainwin.show_all();
	Gtk.main();
}

public class MainWin : Window 
{
	private ListStore model;
	private struct TranscriptData
	{
		public string command;
		public string transcript_text;
		public string expected_text;
	}
	private TranscriptData[] data = {
		TranscriptData() { 
			command = "- start- ",
			transcript_text = """The Crash of 2011
A Titanic epic by Michael Baum
Release 1 / Serial number 110111 / Inform 7 build 6G60 (I6/v6.32 lib 6/12N) SD

Zabar's Main Floor
Zabar's is both the first  and last word in deli excess. Here in the main room of this culinary institution you are nasally assaulted from all sides with such offerings as Bilinski's All Natural Apple Chardonnay Chicken Sausage, Moroccan Oil-Cured, Whole Olives, Handsliced Pastrami, Di Parma Ham, and Bianco Sottobosco. Not to mention the bewildering array of bespoke coffee blends.

A somewhat bedraggled menu bearing the mark of a Manolo Blahnik sole, lies on the floor.

A kosher pickle is on the floor.

>""",
			expected_text = """The Crash of 2011
A Titanic epic by Michael Baum
Release 1 / Serial number 110111 / Inform 7 build 6G60 (I6/v6.32 lib 6/12N) SD

Zabar's Main Floor
Zabar's is both the first  and last word in deli excess. Here in the main room of this culinary institution you are nasally assaulted from all sides with such offerings as Bilinski's All Natural Apple Chardonnay Chicken Sausage, Moroccan Oil-Cured, Whole Olives, Handsliced Pastrami, Di Parma Ham, and Bianco Sottobosco. Not to mention the bewildering array of bespoke coffee blends.

A somewhat bedraggled menu bearing the mark of a Manolo Blahnik sole, lies on the floor.

A kosher pickle is on the floor.

>""" },
		TranscriptData() { 
			command = "look at pickle", 
			transcript_text = "A crisp, briny kosher pickle in near-mint condition.",
			expected_text = "A crisp, briny kosher pickle in mint condition." 
		}
	};

	public MainWin()
	{
		model = new ListStore(3, typeof(string), typeof(string), typeof(string));
		foreach(TranscriptData d in data) {
			TreeIter iter;
			model.append(out iter);
			model.set(iter, 0, d.command, 1, d.transcript_text, 2, d.expected_text); 
		}
		
		var scrolled = new ScrolledWindow(null, null);
		var view = new TreeView.with_model(model);
		var renderer = new CellRendererTranscript();
		var column = new TreeViewColumn.with_attributes("", renderer,
			"command", 0,
			"transcript_text", 1,
			"expected_text", 2);

		renderer.wrap_width = 50;
		view.append_column(column);
		scrolled.add(view);
		add(scrolled);
		set_default_size(480, 400);
		
		destroy.connect(main_quit);
	}
}

public class CellRendererTranscript : CellRenderer
{
	/* Renderer state; any call to render() must yield a cell of the
	same size for the same values of these properties */
	public uint wrap_width { get; set; }
	public string command { get; set; }
	public string transcript_text { get; set; }
	public string expected_text { get; set; }
	
	// Constructor
	public CellRendererTranscript() {
	}
	
	public override void get_size(Widget widget, Gdk.Rectangle? cell_area, out int x_offset, out int y_offset, out int width, out int height)
	{
		if(cell_area != null) {
			x_offset = 0;
			y_offset = 0;
			width = (int)wrap_width;
			height = 20;
		}
	}
	
	public override void render(Gdk.Window window, Widget widget, Gdk.Rectangle background_area, Gdk.Rectangle cell_area, Gdk.Rectangle expose_area, CellRendererState flags)
	{
		paint_box(widget.style, window, StateType.NORMAL, ShadowType.NONE, cell_area, null, null, cell_area.x, cell_area.y, cell_area.width, cell_area.height);
	}
}

