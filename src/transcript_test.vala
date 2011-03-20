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

		view.append_column(column);
		view.size_allocate.connect_after( (view, allocation) => {
			if(renderer.wrap_width != allocation.width) {
				renderer.wrap_width = allocation.width;
				column.queue_resize();
			}
		});
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
	public uint wrap_width { get; set; default = 400; }
	public string command { get; set; }
	public string transcript_text { get; set; }
	public string expected_text { get; set; }
	
	// Constructor
	public CellRendererTranscript() {
	}
	
	public override void get_size(Widget widget, Gdk.Rectangle? cell_area, out int x_offset, out int y_offset, out int width, out int height)
	{
		Pango.Rectangle command_rect, transcript_rect, expected_rect;
		var layout = widget.create_pango_layout(command);
		layout.get_pixel_extents(null, out command_rect);
		layout = widget.create_pango_layout(transcript_text);
		layout.set_width((int)wrap_width / 2 * Pango.SCALE);
		layout.set_wrap(Pango.WrapMode.WORD_CHAR);
		layout.get_pixel_extents(null, out transcript_rect);
		layout = widget.create_pango_layout(expected_text);
		layout.set_width((int)wrap_width / 2 * Pango.SCALE);
		layout.set_wrap(Pango.WrapMode.WORD_CHAR);
		layout.get_pixel_extents(null, out expected_rect);
		
		var calc_width = wrap_width + xpad * 2;
		var calc_height = command_rect.height + uint.max(transcript_rect.height, expected_rect.height) + ypad * 2;
		
		if(cell_area != null) {
			if(&width != null)
				width = int.max(cell_area.width, (int)calc_width);
			if(&height != null)
				height = int.max(cell_area.height, (int)calc_height);
		} else {
			if(&width != null)
				width = (int)calc_width;
			if(&height != null)
				height = (int)calc_height;
		}
		
		if(&x_offset != null)
			x_offset = 0;
		if(&y_offset != null)
			y_offset = 0;
	}
	
	public override void render(Gdk.Window window, Widget widget, Gdk.Rectangle background_area, Gdk.Rectangle cell_area, Gdk.Rectangle expose_area, CellRendererState flags)
	{
		// Make copies of the widget's style
		var command_style = widget.style.clone();
		command_style.rc_style = widget.style.rc_style;
		var good_style = widget.style.clone();
		good_style.rc_style = widget.style.rc_style;
		var bad_style = widget.style.clone();
		bad_style.rc_style = widget.style.rc_style;
		
		// Set the background colors of the styles
		Gdk.Color command_bg, green_bg, red_bg;
		Gdk.Color.parse("#E0E0FF", out command_bg);
		command_style.bg[StateType.NORMAL] = command_bg;
		Gdk.Color.parse("#E0FFE0", out green_bg);
		good_style.bg[StateType.NORMAL] = green_bg;
		Gdk.Color.parse("#FFE0E0", out red_bg);
		bad_style.bg[StateType.NORMAL] = red_bg;
		
		// Attach the styles to the window
		command_style = command_style.attach(window);
		good_style = good_style.attach(window);
		bad_style = bad_style.attach(window);
		
		int x, y, width, height;
		get_size(widget, cell_area, out x, out y, out width, out height);
		x += cell_area.x;
		y += cell_area.y;
		
		Pango.Rectangle command_rect;
		var layout = widget.create_pango_layout(command);
		layout.get_pixel_extents(null, out command_rect);
		
		paint_flat_box(command_style, window, StateType.NORMAL, ShadowType.NONE, cell_area, widget, null, x, y, width, command_rect.height);
		paint_layout(widget.style, window, StateType.NORMAL, true, cell_area, widget, null, x, y, layout);
		
		weak Style transcript_style;
		if(expected_text != null && transcript_text != expected_text)
			transcript_style = bad_style;
		else
			transcript_style = good_style;

		paint_flat_box(transcript_style, window, StateType.NORMAL, ShadowType.NONE, cell_area, widget, null, x, y + command_rect.height, width / 2, height - command_rect.height);
		layout = widget.create_pango_layout(transcript_text);
		layout.set_width((int)wrap_width / 2 * Pango.SCALE);
		layout.set_wrap(Pango.WrapMode.WORD_CHAR);
		paint_layout(transcript_style, window, StateType.NORMAL, true, cell_area, widget, null, x, y + command_rect.height, layout);
		
		paint_flat_box(transcript_style, window, StateType.NORMAL, ShadowType.NONE, cell_area, widget, null, x + width / 2, y + command_rect.height, width / 2, height - command_rect.height);
		layout = widget.create_pango_layout(expected_text);
		layout.set_width((int)wrap_width / 2 * Pango.SCALE);
		layout.set_wrap(Pango.WrapMode.WORD_CHAR);
		paint_layout(transcript_style, window, StateType.NORMAL, true, cell_area, widget, null, x + width / 2, y + command_rect.height, layout);
	}
}

