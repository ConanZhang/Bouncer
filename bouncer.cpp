/*
 * Authors: Charles Khong & Conan Zhang
 * Date: March 25, 2015
 *
 * Description:
 *      1. Decodes a .jpg image for a background
 *      2. Scales the image to RGB24
 *      3. Draws a moving sphere on the image
 *      4. Encodes the image
 *      5. Repeat 3 and 4 300 times
 */

#include <iostream>
#include "bouncer.h"

/**
 * Encodes a single frame and saves it to an MPFF file format.
 * Encode borrowed from official documentation decoding_encoding.c example.
 */
void encode_as_mpff(AVFrame *frame, AVCodecContext *mpff, char* filename)
{
  FILE *f; 

  // Open the file 
  f = fopen(filename, "wb");
  if (!f)
    {
      std::cout << "Could not open test.mpff" << std::endl;
      return;
    }

  // Initialize packet
  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;

  // Encode the image data from frame using the context into packet
  int got_output;
  int ret = avcodec_encode_video2(mpff, &pkt, frame, &got_output);
  if (ret < 0)
    {
      std::cout << "Error encoding" << std::endl;
      return;
    }

  // If successfully encoded, write the frame to file
  if (got_output)
    {
      fwrite(pkt.data, 1, pkt.size, f);
      av_free_packet(&pkt);
    }

  // Close and free resources
  fclose(f);
}

/**
 * Calculates distance between two points.
 */
double dist_from(int x1, int y1, int x2, int y2)
{
  // Divide by 3 because a pixel is made of 3 bytes for RGB24
  double x_dist = (x1-x2)/3;
  double y_dist = y1-y2;
  double distance = sqrt(pow(x_dist,2) + pow(y_dist,2));
  return distance;
}

/**
 * Check to see if a point is with the given radius.
 */
bool inCircle(int x, int y, int centerX, int centerY, int radius)
{
  // Divide by 3 because a pixel is made of 3 bytes for RGB24
  double xDist = (x-centerX)/3;
  double yDist = y-centerY;

  double distance = dist_from(x,y,centerX,centerY);
  if (distance < radius)
    return true;
  else 
    return false;
}

/**
 * Move center coordinate up or down.
 */
int moveCenter(int centerY, bool up)
{
  if (up)
      centerY -= 5;
  else
    centerY +=5;
  return centerY;

}

/**
 *
 */
void drawSphere(AVFrame* orig, int centerX, int centerY, int radius)
{
  // All y pixels
  for (int y = 0; y < orig->height; y++)
    {
      // All x RGB bytes 
      for (int x = 0; x < orig->width*3; x+=3) 
	{
          // Draw black border for circle
	  if (inCircle(x, y, centerX*3, centerY, radius+1))
	    {
              // R
	      orig->data[0][y * orig->linesize[0] + x] = 0;
              // G
	      orig->data[0][y * orig->linesize[0] + x+1] = 0;
              // B
	      orig->data[0][y * orig->linesize[0] + x+2] = 0;
	    }

          // Draw gradient for shading of sphere
          // Gradient is a darker shade of red the further you are from the center 
	  if (inCircle(x, y, centerX*3, centerY, radius))
	    {
	      double dist = dist_from(x,y, centerX*3, centerY);

              // R
	      orig->data[0][y * orig->linesize[0] + x] = 255;
              // G
	      orig->data[0][y * orig->linesize[0] + x+1] = (int)255 -  255 * dist/radius;
              // B
	      orig->data[0][y * orig->linesize[0] + x+2] = (int) 255 - 255 * dist/radius;
	    }
	}
    }
}

int main(int argc, char *argv[])
{
    /* Argument Error Checking */
    //Make sure user only provides one argument
    if(argc != 2)
    {
        std::cout << "Please provide one and only one argument." << std::endl;
        return 0;
    }

    //Get extension from file_name
    std::string file_name(argv[1]);
    std::string jpg_extension = file_name.substr(file_name.length()-4 , 4); 
    std::string jpeg_extension = file_name.substr(file_name.length()-5 , 5); 

    //Make sure user provided a .jpg file
    if(jpg_extension != ".jpg" && jpg_extension != ".JPG" &&  jpeg_extension != ".jpeg"  &&  jpeg_extension != ".JPEG")
    {
        std::cout << "Please provide a .jpg or .jpeg image." << std::endl;
	return 0;
    }


    /**
     * Code to decode and scale image from Dranger
     */

    /* Decode JPG Image */
    // Registers all file formats and codecs
    av_register_all();
    
    // Opens the image file
    AVFormatContext *fmt_context = NULL;

    if (avformat_open_input(&fmt_context, argv[1], NULL, 0) !=0)
      {      
	std::cout << "The image could not be opened." << std::endl;
	return 0;
      }

    // Get the stream info
    if (avformat_find_stream_info(fmt_context, NULL)<0)
      {
	std::cout << "Could not find the stream info for the file." << std::endl;
	return 0;
      }

    // Context for JPG
    AVCodecContext *jpg_context = NULL;
    jpg_context = fmt_context->streams[0]->codec;

    // Find the decoder for JPG
    AVCodec *jpg_codec = NULL;
    jpg_codec = avcodec_find_decoder(jpg_context->codec_id);
    if (jpg_codec == NULL)
      {
	std::cout << "Codec not found" << std::endl;
	return 0;
      }

    // Open JPG codec
    if(avcodec_open2(jpg_context, jpg_codec, NULL)<0)
      {
	std::cout << "Could not open the codec" << std::endl;
	return 0;
      }

    // Allocate resources for frame
    AVFrame *frame = NULL;
    frame = av_frame_alloc();
    if (frame == NULL)
      {
	std::cout << "Could not allocate memory for AVFrame" << std::endl;
	return 0;
      }

    // Allocate frame for scaling to RGB24
    AVFrame *frameRGB24 = NULL;
    frameRGB24 = av_frame_alloc();

    if (frameRGB24 == NULL)
      {
	std::cout << "Could not allocate memory for RGB24 AVFrame" << std::endl;
	return 0;
      }
    
    // Find the MPFF codec
    AVCodec *mpff_codec;
    AVCodecContext *mpff_context = NULL;

    mpff_codec = avcodec_find_encoder(AV_CODEC_ID_MPFF);
    if (!mpff_codec)
      {
	std::cout << "Could not find codec" << std::endl;
	return 0;
      }
    
    // Allocate the MPFF codec for the context
    mpff_context = avcodec_alloc_context3(mpff_codec);
    if (!mpff_context)
      {
	std::cout << "Could not allocate codec context" << std::endl;
	return 0;
      }

    // Set up context and frame properties
    mpff_context->width = jpg_context->width;
    mpff_context->height = jpg_context->height;
    mpff_context->pix_fmt = PIX_FMT_RGB24;
    
    frameRGB24->format = mpff_context->pix_fmt;
    frameRGB24->width =  mpff_context->width;
    frameRGB24->height = mpff_context->height;

    // Open the codec
    if (avcodec_open2(mpff_context, mpff_codec, NULL) < 0)
      {
	std::cout << "Could not open codec" << std::endl;
	return 0;
      }

    // Fill frame with image
    int frameFinished, numBytes;
    uint8_t *buffer = NULL;

    numBytes = avpicture_get_size(mpff_context->pix_fmt, jpg_context->width, jpg_context->height);
    buffer = (uint8_t *) av_malloc(numBytes*sizeof(uint8_t));
   
    avpicture_fill((AVPicture *)frameRGB24, buffer, mpff_context->pix_fmt, jpg_context->width, jpg_context->height);





    /* Scaling image to RGB24 */
    struct SwsContext *sws_context = NULL;
    // Convert the input's pixel format to ours
    sws_context = sws_getContext(jpg_context->width,
				 jpg_context->height,
				 jpg_context->pix_fmt,
				 jpg_context->width,
				 jpg_context->height,
				 PIX_FMT_RGB24,
                                 SWS_BILINEAR,
				 NULL,
				 NULL,
				 NULL);
    if(sws_context == NULL)
      {
	std::cout << "Can't initialize SWS context." << std::endl;
	return 0;
      }

    // Read image into packet the
    AVPacket packet;
    while (av_read_frame(fmt_context, &packet) >= 0)
      {
	if (packet.stream_index != 0)
	  continue;
        // Decode packet into frame and scale it
        avcodec_decode_video2(jpg_context, frame, &frameFinished, &packet);
	if (frameFinished)
	  {
	      sws_scale(sws_context, (uint8_t const * const *)frame->data, frame->linesize, 0, jpg_context->height, frameRGB24->data, frameRGB24->linesize);
	  }

        // Free resources
	av_free_packet(&packet);
      }
    /* End borrowed code */



    /* Draw Sphere */
    int centerY = frameRGB24->height/2;
    bool goingUp = false;
    int radius = frameRGB24->height/10;

    // Draw moving sphere for 300 frames
    for (int i = 0; i < 300; i++)
      {
	// Get a copy of the original frame
	AVFrame* copy = NULL;
	copy = av_frame_alloc();
	copy = av_frame_clone(frameRGB24);

	// Move the center either up or down
	centerY = moveCenter(centerY, goingUp);
	if (centerY < 0 + radius)
	  goingUp = false;
	else if (centerY > copy->height-radius)
	  goingUp = true;

	// Draw the circle on the copy of the frame
	drawSphere(copy, copy->width/2, centerY, radius);

        // Make filename for current image
	char filename[64] = "frame";
	char int_string[32];
	char char_zero[32] = "0";
	sprintf(int_string, "%d", i);

        // Number image from 000 to 299
	if (i < 10)
	  {
	    strcat(filename, char_zero);
	    strcat(filename, char_zero);
	    strcat(filename, int_string);		   
	  }
	else if (i < 100)
	  {
	    strcat(filename, char_zero);
	    strcat(filename, int_string);
	  }
	else 
	  {
	    strcat(filename, int_string);
	  }

        // Append MPFF extension
	strcat(filename, ".mpff");

	// Encode the image data as an MPFF
	encode_as_mpff(copy, mpff_context, filename);

        // Free copy frame resources
	av_free(copy);
      }

    // Free all resources remaining
    av_free(buffer);
    av_free(frame);
    av_free(frameRGB24);
    avcodec_close(mpff_context);
    avcodec_close(jpg_context);
    av_free(mpff_context);
    av_free(jpg_context);
    avformat_close_input(&fmt_context);

    return 0;
}
