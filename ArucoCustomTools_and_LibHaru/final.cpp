// C
#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <windows.h>
#include <intrin.h>
//#include <ctype.h>
//#include <string.h>

// C++
#include <iostream>
#include <typeinfo>
#include <string>

// OpenCV
#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/imgcodecs.hpp>

// Aruco
#include "highlyreliablemarkers.h"
#include "board.h"

// LibHaru
#include "hpdf.h"

// Constants
# define IN_TO_CM_RATIO 2.54
# define DPI_PRINT 300.0  // V TAKOMTO ROZLISENI TREBA TLACIT
# define DPI_PAGE 72.0    // TOTO JE DEFAULT V LIBHARU
# define MARKER_PICTURE_JPG_SIZE_CM 5.0 // POUZITIE KVOLI KVALITE (OBR Z KTOREHO JE VYGENEROVANE PDF MOZE BYT VACSI AKO OBR V PDF, TYM PADOM NIEJE PISMO KOSTRBATE)

// Declarations
void showMeImgDbg(cv::Mat imgObj, std::string imgName, bool debug, bool wait, bool destroyWindow);
void mouseEvent(int event, int x, int y, int flags, void* param);

float cm2in(float cm);
float in2cm(float in);
float in2px(float in, float dpi);
float px2in(float px, float dpi);

void split(const string &s, char delim, vector<string> &elems);
vector<string> split(const string &s, char delim);

string char2string(char c);
string int2string(int i);


float a4_page_width_cm = 21;
float a4_page_height_cm = 29.7;


// COLORS
cv::Scalar color_white = cv::Scalar(255);
cv::Scalar color_black = cv::Scalar(0);   // ??? cv::Scalar(0, 0, 0)


// TODO: spravit lepsie napr cez pomer a pod.
int font_thickness = 3;
int font_horizontal_align = 50;
int font_vertical_align = 10;
double font_size = 1.5;
int border_mat_thickness = 20;




// LIBHARU
jmp_buf env;
#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler(HPDF_STATUS error_no, HPDF_STATUS   detail_no, void *user_data){
	printf("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
		(HPDF_UINT)detail_no);
	longjmp(env, 1);
}



int main(int argc, char **argv) {

#ifdef __WIN32__
	string FILE_SEPARATOR = "\\";
#else
	string FILE_SEPARATOR = "/";
#endif
	/////////////////////////////////////////////////////////////////////////

	// PARAMS
	// WORKING:
	// reduced-dict-plastic.yml output.pdf 0 -1 17 101-105	    // DEFAULT BORDER
	// reduced-dict-plastic.yml output.pdf 1.5 -1 3 0-5		    // DEFAULT BORDER
	// reduced-dict-plastic.yml output.pdf 1.5 -1 3 1,5,9-12,14 // DEFAULT BORDER

	// WORKS:
	// reduced-dict-plastic.yml output.pdf 0 0 20 101-105 // JEDEN 20 CM MARKER NA KAZDEJ STRANE BEZ OKRAJOV => BEZ ID
	 

	// TODO ODMERAT:
	// reduced-dict-plastic.yml output.pdf 0 1 17 1,5,9-12,14
	// reduced-dict-plastic.yml output.pdf 0 3 17 1,5,9-12,14

	// WORKING (TESTED WITH ALL BORDERS):
	// reduced-dict-plastic.yml output.pdf 0 -1 3 101-105 	// DEFAULT BORDER
	// reduced-dict-plastic.yml output.pdf 0 0 3 101-105 	// ZERO BORDER
	// reduced-dict-plastic.yml output.pdf 0 3 3 101-105 	// CUSTOM BORDER

	//	// http://www.cplusplus.com/forum/windows/6457/
	//	// char buffer[MAX_PATH];//always use MAX_PATH for filepaths
	//	// GetModuleFileName(NULL, buffer, sizeof(buffer));
	//	// cout << "Filepath:" << buffer << "\n";
	//	// std::getchar();

	if (argc != 7) {
		std::cerr << "Invalid number of arguments" << std::endl;
		std::cerr << "Usage: dictionary.yml output.pdf 'page margin' 'marker border' 'marker size' 'marker range'" << std::endl;
		std::cerr << "1. dictionary.yml: input dictionary from where markers are taken to create the board" << std::endl;
		std::cerr << "2. output.pdf:     output pdf file for the created board" << std::endl;
		std::cerr << "3. page margin:    margin of A4 page [cm]" << std::endl;
		std::cerr << "4. marker border:  border of marker [cm] (<1 == default (size of 1 marker bit), 0 == zero border, >1 == custom border) " << std::endl;
		std::cerr << "5. marker size:    size of marker [cm]" << std::endl;
		std::cerr << "6. marker id:      markers to be printed e.g. 1,3,9-11,16" << std::endl;
		exit(-1);
	}

	std::string dictionaryfile = argv[1];
	char * pdfFileName = argv[2];
	float page_margin = atof(argv[3]);
	int marker_border_size_cm = atoi(argv[4]);
	float marker_picture_size_cm = atof(argv[5]); // float img_cm = atof(argv[4]);
	string print_markers(argv[6]);
	//print_markers = "1,5,9-12,14";

	float lmargin_cm = page_margin; // left margin
	float rmargin_cm = page_margin; // right margin
	float tmargin_cm = page_margin; // top margin
	float bmargin_cm = page_margin; // bottom margin
	/////////////////////////////////////////////////////////////////////////
	/*

	for (int i = 0; i < argc; ++i) {
	std::cout << "argv[" << i << "]: " <<
	argv[i] << std::endl;
	}

	std::cout << "argc: " << argc << std::endl;

	*/
	/////////////////////////////////////////////////////////////////////////
	vector<string> digits_and_ranges = split(print_markers, ',');
	vector<string> range;
	vector<string> markers_id_to_print;

	for (int i = 0; i < digits_and_ranges.size(); i++){
		if (digits_and_ranges[i].find("-") != string::npos){
			range = split(digits_and_ranges[i], '-');
			if ((std::stoi(range[0], nullptr) < std::stoi(range[1], nullptr)) ? true : false){
				for (int j = 0; j <= std::stoi(range[1], nullptr) - std::stoi(range[0], nullptr); j++){
					markers_id_to_print.push_back(int2string(std::stoi(range[0], nullptr) + j));
				}
			}
			else{
				std::cout << "Wrong range: " << range[0] << " - " << range[1] << std::endl;
			}
		}
		else{
			markers_id_to_print.push_back(digits_and_ranges[i]);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	aruco::Dictionary D;
	D.fromFile(dictionaryfile);
	if (D.size() == 0) {
		std::cerr << "Error: Dictionary is empty" << std::endl;
		exit(-1);
	}



	//unsigned int marker_size_px = cm2in(MARKER_PICTURE_JPG_SIZE_CM) * DPI_PRINT;
	unsigned int marker_size_px = in2px(cm2in(MARKER_PICTURE_JPG_SIZE_CM), DPI_PRINT);
	cv::Mat marker = D[0].getImg(marker_size_px);
	// musi platit: (marker.rows + 2) % D[0].n() == 0 V OPACNOM PRIPADE SA VYGENERUJE PRAZDNY MARKER
	// D[0].n()
	marker_size_px = marker.rows;
	int marker_picture_size_bits = D[0].n() + 2; // pocet bitov markeru + cierne okraje vramci markeru
	int marker_border_size_bits = D[0].n() + 4;  // pocet bitov markeru + cierne okraje vramci markeru + biele okraje okolo markeru
	int marker_bit_size_px = marker_size_px / marker_picture_size_bits;
	

	int marker_marker_distance_px;
	float pdf_img_size;
	if (marker_border_size_cm < 0){ // DEFAULT VZDIALENOST MEDZI MARKERMI SU 2 BITY (1 BIT OD KAZDEHO OKRAJA)
		marker_marker_distance_px = 2 * marker_bit_size_px; 

		// REALNA VELKOST MARKERU BEZ OKRAJA (K VELKOSTI MARKERU SA PRIRATA ESTE OKRAJ)
		pdf_img_size = (marker_border_size_bits * marker_picture_size_cm) / (float)marker_picture_size_bits;

		// REALNA VELKOST MARKERU VRATANE OKRAJU (SAMOTNY MARKER BUDE TYM PADOM MENSI) - DEBUG PURPOSES ONLY
		// pdf_img_size = marker_picture_size_cm;
	}
	else if (marker_border_size_cm == 0) // NULOVA VZDIALENOST
	{
		marker_marker_distance_px = 0;
		pdf_img_size = marker_picture_size_cm;
	}
	else // VLASTNA VZDIALENOST
	{
		marker_marker_distance_px = marker_border_size_cm * marker_size_px / marker_picture_size_cm;  // VLASTNA VZDIALENOST
		pdf_img_size = marker_border_size_cm + marker_picture_size_cm;
	}


	// ak ma marker napr. 7 bitov tak vyzera tak ze ma 7 stlpcov a 7 riadkov plus z kazdej strany ma jeden cierny bit (stvorcek) kde uz nemozu byt biele bity (stvorceky)
	if (marker_picture_size_bits > marker_size_px){
		std::cerr << "Small physical size of marker using " << D[0].n() << "bits" << std::endl;
		exit(-1);
	}


	// CREATE BORDER MATRIX
	cv::Mat border_mat(marker_size_px + marker_marker_distance_px, marker_size_px + marker_marker_distance_px, CV_8UC1);

	// CREATE SUB-RECTANGLE REPRESENTING MARKER
	cv::Rect subrect_area = cv::Rect((border_mat.cols - marker.cols) / 2,
		(border_mat.rows - marker.rows) / 2,
		marker_size_px,
		marker_size_px);

	// POINT SUB-RECTANGLE INSIDE BORDER MATRIX
	cv::Mat subrect_mat(border_mat, subrect_area);

	// SET MARKER ID POSITION
	cv::Point text_pos = cv::Point((border_mat.cols - marker.cols) / 2 - font_horizontal_align,
		(border_mat.rows - marker.rows) / 2 - font_vertical_align);

	// CREATE RECT REPRESENTING BORDER
	cv::Rect border_rect = cv::Rect(0, 0, border_mat.cols, border_mat.rows);



	string imgFileName = "";
	int dict_size = D.size();
	bool default_val = false;
	std::vector<bool> use_id(dict_size, default_val);

	for (int i = 0; i < markers_id_to_print.size(); i++){
		use_id[atoi(markers_id_to_print[i].c_str())] = true;
	}


	for (int i = 0; i < dict_size; i++){
		if (use_id[i]){
			// SET COLOR FOR BORDER MATRIX
			border_mat.setTo(color_white);

			// SET BORDER FOR BORDER MATRIX
			cv::rectangle(border_mat, border_rect, color_black, border_mat_thickness);

			// SET MARKER ID
			cv::putText(border_mat,		  // image
				std::to_string(i),		  // text
				text_pos,				  // text position
				cv::FONT_HERSHEY_SIMPLEX, // fontFace
				font_size,                // fontScale
				color_black,			  // black color (CV_8UC1)
				font_thickness 	          // thickness
				);

			// GENERATE MARKER IMAGE
			marker = D[i].getImg(marker_size_px);

			// PLACE MARKER IMAGE INSIDE SUBRECT MATRIX
			marker.copyTo(subrect_mat);

			// SAVE OUTPUT IMAGE
			imgFileName = "out_imgs" + FILE_SEPARATOR + std::to_string(i) + ".jpg";
			std::cout << imgFileName << std::endl;
			cv::imwrite(imgFileName,border_mat);

			//std::cout << "border_mat.rows: " << border_mat.rows << std::endl;
			//std::cout << "border_mat.cols: " << border_mat.cols << std::endl;
			//std::cout << "marker.rows: " << marker.rows << std::endl;
			//std::cout << "marker.cols: " << marker.cols << std::endl;
			//std::cout << "###########################" << std::endl;
		}
	}
	std::cout << "###########################" << std::endl;

	/////////////////////////////////////////////////////////////////////////
	// LIBHARU VARIABLES
	HPDF_Doc  pdf;
	HPDF_Font font;
	HPDF_Destination dst;
	HPDF_Image image;

	pdf = HPDF_New(error_handler, NULL);
	if (!pdf) {
		printf("error: cannot create PdfDoc object\n");
		return 1;
	}

	HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
	/////////////////////////////////////////////////////////////////////////
	int nr_x_chess_elements = (a4_page_width_cm - (lmargin_cm + rmargin_cm)) / pdf_img_size;
	int nr_y_chess_elements = (a4_page_height_cm - (tmargin_cm + bmargin_cm)) / pdf_img_size;

	float x_offset_cm;
	float y_offset_cm;

	int nr_aruco_markers = markers_id_to_print.size();  // 200; //20
	int nr_markers_per_page = nr_x_chess_elements * nr_y_chess_elements;


	if (nr_markers_per_page == 0){
		std::cerr << "ERROR" << std::endl;
		std::cerr << "pdf_img_size = (marker_border_size_bits * marker_picture_size_cm) / (float)marker_picture_size_bits" << std::endl;
		std::cerr << "pdf_img_size: " << pdf_img_size << std::endl;
		std::cerr << "marker_border_size_bits: " << marker_border_size_bits << std::endl;
		std::cerr << "marker_picture_size_cm: " << marker_picture_size_cm << std::endl;
		std::cerr << "marker_picture_size_bits: " << marker_picture_size_bits << std::endl;
		std::cerr << "------------------------" << std::endl;
		std::cerr << "nr_x_chess_elements = (a4_page_width_cm - (lmargin_cm + rmargin_cm)) / pdf_img_size" << std::endl;
		std::cerr << "nr_x_chess_elements: " << nr_x_chess_elements << std::endl;
		std::cerr << "a4_page_width_cm: " << a4_page_width_cm << std::endl;
		std::cerr << "lmargin_cm: " << lmargin_cm << std::endl;
		std::cerr << "rmargin_cm: " << rmargin_cm << std::endl;
		std::cerr << "------------------------" << std::endl;
		std::cerr << "nr_y_chess_elements = (a4_page_height_cm - (tmargin_cm + bmargin_cm)) / pdf_img_size" << std::endl;
		std::cerr << "nr_y_chess_elements: " << nr_y_chess_elements << std::endl;
		std::cerr << "a4_page_height_cm: " << a4_page_height_cm << std::endl;
		std::cerr << "tmargin_cm: " << tmargin_cm << std::endl;
		std::cerr << "bmargin_cm: " << bmargin_cm << std::endl;
		std::cerr << "------------------------" << std::endl;
		std::cerr << "nr_markers_per_page = nr_x_chess_elements * nr_y_chess_elements" << std::endl;
		std::cerr << "nr_markers_per_page: " << nr_markers_per_page << std::endl;
		std::cerr << "------------------------" << std::endl;
		exit(-1);
	}


	int nr_pages;
	if (nr_aruco_markers < nr_markers_per_page){
		nr_pages = 1;
	}
	else{
		nr_pages = nr_aruco_markers / nr_markers_per_page;
	}

	int nr_markers_per_last_page = nr_aruco_markers % nr_markers_per_page;

	if (nr_markers_per_last_page > 0 && nr_aruco_markers > nr_markers_per_page){
		nr_pages++;
	}


	// FONT
	font = HPDF_GetFont(pdf, "Helvetica", NULL);
	HPDF_REAL width_of_page;
	HPDF_REAL height_of_page;
	char str[15];

	std::vector<HPDF_Page> pages(nr_pages);
	for (int i = 0; i < pages.size(); i++){

		/* create page1 in pdf document */
		pages[i] = HPDF_AddPage(pdf);

		/* set page size */
		HPDF_Page_SetSize(pages[i], HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

		/* resolution set */
		HPDF_Page_Concat(pages[i], DPI_PAGE / DPI_PRINT, 0, 0, DPI_PAGE / DPI_PRINT, 0, 0);

		// FONT		
		// sprintf(str, "%d", i);

		height_of_page = HPDF_Page_GetHeight(pages[i]);
		width_of_page = HPDF_Page_GetWidth(pages[i]);

		HPDF_Page_BeginText(pages[i]);
		HPDF_Page_SetFontAndSize(pages[i], font, 200);
		// HPDF_Page_TextOut(pages[i], (width_of_page - 5) / 2, height_of_page - 10, str);
		HPDF_Page_TextOut(pages[i], (width_of_page - 5) / 2, height_of_page - 10, markers_id_to_print[i].c_str());
		HPDF_Page_EndText(pages[i]);



	}
	/////////////////////////////////////////////////////////////////////////
	std::cout << "###########################" << std::endl;
	cout << "dictionary file: " << dictionaryfile << endl;
	cout << "output PDF: " << pdfFileName << endl;
	cout << "page margin [cm]: " << page_margin << endl;
	cout << "marker border size [cm]: " << marker_border_size_cm << endl;
	cout << "marker size (black part) [cm]: " << marker_picture_size_cm << endl; // cout << "img_cm: " << img_cm << endl;
	cout << "markers to be printed: " << print_markers << " (" << markers_id_to_print.size() << ")" << endl;
	cout << "a4_page_height_cm: " << a4_page_height_cm << endl;
	cout << "a4_page_width_cm: " << a4_page_width_cm << endl;
	cout << "nr_pages: " << nr_pages << endl;
	cout << "nr_y_chess_elements: " << nr_y_chess_elements << endl;
	cout << "nr_x_chess_elements: " << nr_x_chess_elements << endl;
	cout << "nr_aruco_markers: " << nr_aruco_markers << endl;
	std::cout << "###########################" << std::endl;
	/////////////////////////////////////////////////////////////////////////





	int marker_counter = 0;
	x_offset_cm = lmargin_cm; // vykreslovanie zlava do prava
	y_offset_cm = tmargin_cm; // vykreslovanie zhora dole
	for (int page = 0; page < nr_pages; page++){
		for (int y = 0; y < nr_y_chess_elements; y++){
			for (int x = 0; x < nr_x_chess_elements; x++){
				if (marker_counter < nr_aruco_markers){
					imgFileName = "out_imgs" + FILE_SEPARATOR + markers_id_to_print[marker_counter] + ".jpg";
					image = HPDF_LoadJpegImageFromFile(pdf, imgFileName.c_str());

					float x_img_pos = x * in2px(cm2in(pdf_img_size), DPI_PRINT) + in2px(cm2in(x_offset_cm), DPI_PRINT);
					float y_img_pos = in2px(cm2in(a4_page_height_cm), DPI_PRINT) - (y + 1) * in2px(cm2in(pdf_img_size), DPI_PRINT) - in2px(cm2in(y_offset_cm), DPI_PRINT);
					float x_img_size = 1 * in2px(cm2in(pdf_img_size), DPI_PRINT);
					float y_img_size = 1 * in2px(cm2in(pdf_img_size), DPI_PRINT);

					HPDF_Page_DrawImage(pages[page],
						image,
						x_img_pos,
						y_img_pos,
						x_img_size,
						y_img_size
						);

					marker_counter++;
				}
				else{
					goto END_DRAWING;
				}
			}
		}
	END_DRAWING:
		;
	}




	/* save the document to a file */
	HPDF_SaveToFile(pdf, pdfFileName);

	/* clean up */
	HPDF_Free(pdf);

	return 0;

	cv::waitKey(0);
}



// Debug purposes - show image
void showMeImgDbg(cv::Mat imgObj, std::string imgName, bool debug, bool wait, bool destroyWindow){

	// prod
	//cv::imshow(imgName, imgObj);

	// dev
	const char * c = imgName.c_str();
	cv::namedWindow(imgName, 1);
	cvSetMouseCallback(c, mouseEvent, NULL);
	cv::imshow(imgName, imgObj);

	if (debug){
		__debugbreak();
	}

	if (wait){
		cv::waitKey(0);
	}

	if (destroyWindow){
		//cv::destroyAllWindows();
		cv::destroyWindow(imgName);
	}
}


// Debug puposes - show pixel position
void mouseEvent(int event, int x, int y, int flags, void* param)
{
	//IplImage* img = (IplImage*)param;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		//printf("%d,%d\n",x, y);
		std::cout << "[x: " << x << "; y: " << y << "]" << std::endl;
	}
}



// SIZE [IN] = PIXELS / DPI
// SIZE [CM] = PIXELS / DPI * 2.54
// DPI = PIXELS / SIZE [IN]
// PIXELS = SIZE [IN] * DPI

float cm2in(float cm){
	// inches = cm / IN_TO_CM_RATIO
	return (cm / IN_TO_CM_RATIO);
}


float in2cm(float in){
	// cm = inches * IN_TO_CM_RATIO
	return (in * IN_TO_CM_RATIO);
}


float in2px(float in, float dpi){
	// pixels = inches * dpi
	return (in * dpi);
}


float px2in(float px, float dpi){
	// inches = pixels / dpi
	return (px / dpi);
}



string char2string(char c){
	stringstream ss;
	string s;
	ss << c;
	ss >> s;
	return s;
	//ss.str("");
	//ss.clear();
}

string int2string(int i){
	stringstream ss;
	string s;
	ss << i;
	ss >> s;
	return s;
	//ss.str("");
	//ss.clear();
}


void split(const string &s, char delim, vector<string> &elems) {
	stringstream ss;
	ss.str(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
}

vector<string> split(const string &s, char delim) {
	vector<string> elems;
	split(s, delim, elems);
	return elems;
}