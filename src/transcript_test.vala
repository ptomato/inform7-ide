using Gtk;

enum Styl {
	UNPLAYED,
	UNCHANGED,
	CHANGED,
	NO_EXPECTED,
	NO_MATCH,
	NEAR_MATCH,
	EXACT_MATCH,
	COMMAND,
	HIGHLIGHT,
	ACTIVE,
	LAST
}

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
		},
		TranscriptData() {
			command = "etaoin shrdlu",
			transcript_text = "Qwerty qwerty qwerty.",
			expected_text = null
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

		renderer.xpad = renderer.ypad = 6;
		view.size_allocate.connect_after( (view, allocation) => {
			if(renderer.default_width != allocation.width) {
				renderer.default_width = allocation.width;
				column.queue_resize();
			}
		});
		view.append_column(column);
		view.headers_visible = false;
		scrolled.add(view);
		add(scrolled);
		set_default_size(480, 400);

		view.realize();
		
		destroy.connect(main_quit);
	}
}

public class CellRendererTranscript : CellRenderer
{
	// Colors
	private struct TranscriptColor 
	{
		public double r;
		public double g;
		public double b;
	}
	private static TranscriptColor[] colors = 
	{
		TranscriptColor() { r=0.8, g=0.8, b=0.8 }, // UNPLAYED
		TranscriptColor() { r=0.6, g=1.0, b=0.6 }, // UNCHANGED
		TranscriptColor() { r=1.0, g=0.6, b=0.6 }, // CHANGED
		TranscriptColor() { r=0.7, g=0.7, b=0.7 }, // NO_EXPECTED
		TranscriptColor() { r=1.0, g=0.5, b=0.5 }, // NO_MATCH
		TranscriptColor() { r=1.0, g=1.0, b=0.5 }, // NEAR_MATCH
		TranscriptColor() { r=0.5, g=1.0, b=0.5 }, // EXACT_MATCH
		TranscriptColor() { r=0.6, g=0.8, b=1.0 }, // COMMAND
		TranscriptColor() { r=0.4, g=0.4, b=1.0 }, // HIGHLIGHT
		TranscriptColor() { r=1.0, g=1.0, b=0.7 } // ACTIVE
	};
	
	/* Renderer state; any call to render() must yield a cell of the
	same size for the same values of these properties */
	public uint default_width { get; set; default = 400; }
	public uint text_padding { get; set; default = 6; }
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
		var transcript_width = default_width / 2 - xpad;
		layout.get_pixel_extents(null, out command_rect);
		layout = widget.create_pango_layout(transcript_text);
		layout.set_width((int)(transcript_width - text_padding * 2) * Pango.SCALE);
		layout.set_wrap(Pango.WrapMode.WORD_CHAR);
		layout.get_pixel_extents(null, out transcript_rect);
		layout = widget.create_pango_layout(expected_text);
		layout.set_width((int)(transcript_width - text_padding * 2) * Pango.SCALE);
		layout.set_wrap(Pango.WrapMode.WORD_CHAR);
		layout.get_pixel_extents(null, out expected_rect);
		
		var calc_width = default_width;
		var calc_height = command_rect.height + uint.max(transcript_rect.height, expected_rect.height) + ypad * 2 + text_padding * 2;
		
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
	
	private static void set_rgb_style(Cairo.Context cr, Styl style)
	{
		cr.set_source_rgb(CellRendererTranscript.colors[style].r, 
			CellRendererTranscript.colors[style].g, 
			CellRendererTranscript.colors[style].b);
	}
	
	public override void render(Gdk.Window window, Widget widget, Gdk.Rectangle background_area, Gdk.Rectangle cell_area, Gdk.Rectangle expose_area, CellRendererState flags)
	{
		int x, y, width, height;
		get_size(widget, cell_area, out x, out y, out width, out height);
		x += cell_area.x + (int)xpad;
		y += cell_area.y + (int)ypad;
		width -= (int)xpad * 2;
		height -= (int)ypad * 2;
		
		// Decide what state to draw the widget components in
		StateType state;
		switch(flags) {
			case CellRendererState.PRELIT:
				state = StateType.PRELIGHT;
				break;
			case CellRendererState.INSENSITIVE:
				state = StateType.INSENSITIVE;
				break;
			default:
				state = StateType.NORMAL;
				break;
		}
		
		// Draw the rectangles directly on the Cairo context
		var cr = Gdk.cairo_create(window);

		// Draw the command		
		Pango.Rectangle command_rect;
		var layout = widget.create_pango_layout(command);
		layout.get_pixel_extents(null, out command_rect);

		set_rgb_style(cr, Styl.COMMAND);
		cr.rectangle(x, y, 
			width, command_rect.height);
		cr.fill();
		paint_layout(widget.style, window, state, true, cell_area, widget, null, 
			x, y, 
			layout);
		
		// Draw the transcript text
		var transcript_width = default_width / 2 - xpad;
		if(expected_text != null && transcript_text != expected_text)
			set_rgb_style(cr, Styl.CHANGED);
		else
			set_rgb_style(cr, Styl.UNCHANGED);

		cr.rectangle(x, y + command_rect.height, 
			width / 2, height - command_rect.height);
		cr.fill();
		layout = widget.create_pango_layout(transcript_text);
		layout.set_width((int)(transcript_width - text_padding * 2) * Pango.SCALE);
		layout.set_wrap(Pango.WrapMode.WORD_CHAR);
		paint_layout(widget.style, window, state, true, cell_area, widget, null, 
			x + (int)text_padding, 
			y + command_rect.height + (int)text_padding, 
			layout);
		
		// Draw the expected text
		if(expected_text == null)
			set_rgb_style(cr, Styl.NO_EXPECTED);
		else if(transcript_text != expected_text)
			set_rgb_style(cr, Styl.NO_MATCH);
		else
			set_rgb_style(cr, Styl.EXACT_MATCH);
		
		cr.rectangle(x + width / 2, y + command_rect.height, 
			width / 2, height - command_rect.height);
		cr.fill();
		layout = widget.create_pango_layout(expected_text);
		layout.set_width((int)(transcript_width - text_padding * 2) * Pango.SCALE);
		layout.set_wrap(Pango.WrapMode.WORD_CHAR);
		paint_layout(widget.style, window, state, true, cell_area, widget, null, 
			x + width / 2 + (int)text_padding, 
			y + command_rect.height + (int)text_padding, 
			layout);
			
		// Draw some lines
		paint_hline(widget.style, window, state, cell_area, widget, null,
			x, x + width, y + command_rect.height);
		paint_vline(widget.style, window, state, cell_area, widget, null,
			y + command_rect.height, y + height, x + width / 2);
	}
}

