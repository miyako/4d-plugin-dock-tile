//
//  NSBezierPath+CGPath.hpp
//  Dock Tile
//
//  Created by Eric Marchand on 24/07/2021.
//

#ifndef DOCK_PATH_H
#define DOCK_PATH_H

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@interface NSBezierPath (CGPath)

// No `CGPath` property here: AppKit declares its own -[NSBezierPath CGPath]
// since the macOS 14 SDK (available on macOS 14+ only), and `path.CGPath`
// would bind to it and fail on macOS 11-13. Call quartzPath explicitly.
// Returns a +1 CGPathRef (or NULL for an empty path): CGPathRelease it.
-(CGPathRef)quartzPath CF_RETURNS_RETAINED;

@end

@interface ProgressCircleShapeLayer : CAShapeLayer

- (instancetype)initWithRadius:(CGFloat)radius center:(CGPoint)center;
- (void)setProgress:(CGFloat)progress;

@end

@interface ProgressSquircleShapeLayer : CAShapeLayer

- (instancetype)initWithRect:(CGRect)rect;
- (void)setProgress:(CGFloat)progress;

@end


#endif /* DOCK_PATH_H */
