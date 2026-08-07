Captured: 2026-08-04. Kept locally so the design does not have to be fetched again.

## Extracted values

### Window
| Property | Value |
| --- | --- |
| Corner radius | 16px (all four corners) |
| Border | 1px solid rgba(255, 255, 255, 0.2) |
| Background | #1e1e1e (`--window-background`) |
| Shadow 1 | 0 0 1px rgba(0, 0, 0, 0.8) |
| Shadow 2 | 0 18px 54px rgba(0, 0, 0, 0.57) |
| Width (frame) | 840px |

### Title bar (`Title Bar`, node 4365:31973)
| Property | Value |
| --- | --- |
| Padding | 8px on all sides |
| Gap (controls → title) | 16px |
| Alignment | items-center, content starts at left |
| Height | 8 + 14 + 8 = **30px** |
| Background | rgba(31, 31, 31, 0.55) |
| Backdrop blur | 3px |
| Bottom border | 0.67px solid rgba(255, 255, 255, 0.1) |

### Window controls (`Window Controls`, node 4365:31974)
| Property | Value |
| --- | --- |
| Gap between buttons | 9px |
| Padding right | 8px |
| Total width | 3x14 + 2x9 + 8 = **68px** |
| Button size | 14x14px |
| Button radius | full (100px) |
| Button border | 0.5px solid rgba(0, 0, 0, 0.45) |
| Close fill | #ff5c60 |
| Minimize fill | #fac800 |
| Zoom fill | #35c759 |

Resulting caption offset: 8 (padding) + 68 (controls) + 16 (gap) = **92px** from the window edge.

### Title (`Title`, node 4365:31975)
| Property | Value |
| --- | --- |
| Font | SF Pro Bold, `wdth` 100 |
| Size | 13px |
| Line height | 15px |
| Color | rgba(255, 255, 255, 0.85) (`--labels/primary`) |
| Blend mode | plus-lighter |
| Alignment | left, immediately after the controls |

SF Pro is not installed here; pearOS ships **Mutern VF** as its stand-in.

### Content area (`Content Area`, node 4365:31976)
Height 369px, vertical scrollbar 12px wide, thumb 6px wide, radius 1000px,
color rgba(255, 255, 255, 0.55), inset 3px top/bottom. Not part of the decoration.

## Original response from `get_design_context`

```tsx
type WindowsComposedProps = {
  className?: string;
  active?: "True";
  children?: React.ReactNode | null;
  mode?: "Dark";
  windowType?: "Default + Title";
};

function WindowsComposed({ className, active = "True", children = null, mode = "Dark", windowType = "Default + Title" }: WindowsComposedProps) {
  return (
    <div className={className || "bg-[var(--window-background,#1e1e1e)] border border-[rgba(255,255,255,0.2)] border-solid content-stretch flex flex-col items-start overflow-clip relative rounded-[16px] shadow-[0px_0px_1px_0px_rgba(0,0,0,0.8),0px_18px_54px_0px_rgba(0,0,0,0.57)] w-[840px]"} data-node-id="4365:31971">
      <div className="content-stretch flex gap-[16px] items-center p-[8px] relative shrink-0 w-full" data-node-id="4365:31973" data-name="Title Bar">
        <div className="absolute backdrop-blur-[3px] bg-[rgba(31,31,31,0.55)] border-[rgba(255,255,255,0.1)] border-b-[0.67px] border-solid inset-[0_0_-1px_0]" data-node-id="4365:31972" data-name="Scroll Edge Effect - Hard" />
        <div className="content-stretch flex gap-[9px] items-center pr-[8px] relative shrink-0" data-node-id="4365:31974" data-name="Window Controls">
          <div className="bg-[#ff5c60] border-[0.5px] border-[rgba(0,0,0,0.45)] border-solid relative rounded-[100px] shrink-0 size-[14px]" data-node-id="I4365:31974;4365:36390" data-name="Close" />
          <div className="bg-[#fac800] border-[0.5px] border-[rgba(0,0,0,0.45)] border-solid relative rounded-[100px] shrink-0 size-[14px]" data-node-id="I4365:31974;4365:36391" data-name="Minimize" />
          <div className="bg-[#35c759] border-[0.5px] border-[rgba(0,0,0,0.45)] border-solid relative rounded-[100px] shrink-0 size-[14px]" data-node-id="I4365:31974;4365:36392" data-name="Zoom" />
        </div>
        <div className="content-stretch flex gap-[3px] items-start relative shrink-0" data-node-id="4365:31975" data-name="Title">
          <div className="content-stretch flex flex-col gap-[2px] items-start relative shrink-0" data-node-id="I4365:31975;4358:16851" data-name="Title and Subtitle">
            <p className="[word-break:break-word] font-['SF_Pro:Bold'] font-bold leading-[15px] mix-blend-plus-lighter relative shrink-0 text-[13px] text-[color:var(--labels/primary,rgba(255,255,255,0.85))] whitespace-nowrap" data-node-id="I4365:31975;4358:16852" style={{ fontVariationSettings: '"wdth" 100' }}>
              Title
            </p>
          </div>
        </div>
      </div>
      <div className="content-stretch flex h-[369px] items-start relative shrink-0 w-full" data-node-id="4365:31976" data-name="Content Area">
        <div className="flex-[1_0_0] h-full min-w-px relative" data-node-id="4365:31977" data-name="Content Area">
          {children}
        </div>
        <div className="h-[319px] overflow-clip relative shrink-0 w-[12px]" data-node-id="4365:31978" data-name="Scrollbar - Vertical">
          <div className="-translate-x-1/2 absolute bottom-[3px] left-1/2 top-[3px] w-[6px]" data-node-id="I4365:31978;497:6526" data-name="Frame">
            <div className="-translate-x-1/2 absolute bg-[var(--miscellaneous/scrollbar,rgba(255,255,255,0.55))] bottom-1/2 left-1/2 rounded-[1000px] top-0 w-[6px]" data-node-id="I4365:31978;497:6479" data-name="Thumb" />
          </div>
        </div>
      </div>
    </div>
  );
}
```
