export const TileStatuses = ["Empty", "Owned", "Trail", "Head"] as const;
export type TileStatus = (typeof TileStatuses)[number];

export interface Tile {
  ownerPid: number;
  status: TileStatus;
}
